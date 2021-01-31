// 
// 
// 

#include "Alexa.h"

// Includes for the server
#include <WiFiUdp.h>
#include <HTTPServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

#include "Log.h"
#include "WebSocket.h" //for the IPaddress
#include "RKP.h" //for keypress

bool devValues[]{ false,false};
String devNames[]{ "Home Security", "Home Partset"};
uint8_t currentDeviceCount = sizeof(devValues)/sizeof(devValues[0]);

WiFiUDP espalexaUdp;
IPAddress ipMulti;
bool udpConnected = false;

String deviceJsonString(uint8_t deviceId)
{
	String escapedMac = WebSocket::escapedMac; //this takes a copy
	Serial.println("Using Mac: " + escapedMac);
	
	String buf_lightid = escapedMac.substring(0,10); //first 10 chars
	if(deviceId <= 15)
		buf_lightid += "0";
	buf_lightid += String(deviceId, HEX);
	LogLn("Unique Device: "+ buf_lightid);
	
	String sValue = devValues[deviceId - 1] ? "true" : "false";

	String buf = R"({"state":{"on":)" + sValue 
	+ R"(,"bri":254,"alert":"none","mode":"homeautomation","reachable":true})"
	+ R"(,"type":"Dimmable light","name":")" + devNames[deviceId - 1] + R"(")"
	+ R"(,"modelid":"LWB010","manufacturername":"Philips","productname":"E1")"
	+ R"(,"uniqueid":")" + buf_lightid + R"(")"
	+ R"(,"swversion":"espalexa-2.5.0"})";

	LogLn("==");
	LogLn(buf);
	LogLn("==");

	return buf;
}

void serveDescription(httpsserver::HTTPRequest* req, httpsserver::HTTPResponse* res)
{
	// Status code is 200 OK by default.
	res->setHeader("Content-Type", "text/xml");
	String sIP = WebSocket::sIPAddr;

	String buf = "<?xml version=\"1.0\" ?>"
		"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
		"<specVersion><major>1</major><minor>0</minor></specVersion>"
		"<URLBase>http://" + sIP + ":80/</URLBase>"
		"<device>"
		"<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
		"<friendlyName>Espalexa (" + sIP + ":80)</friendlyName>"
		"<manufacturer>Royal Philips Electronics</manufacturer>"
		"<manufacturerURL>http://www.philips.com</manufacturerURL>"
		"<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
		"<modelName>Philips hue bridge 2012</modelName>"
		"<modelNumber>929000226503</modelNumber>"
		"<modelURL>http://www.meethue.com</modelURL>"
		"<serialNumber>" + WebSocket::escapedMac + "</serialNumber>"
		"<UDN>uuid:2f402f80-da50-11e1-9b23-" + WebSocket::escapedMac + "</UDN>"
		"<presentationURL>index.html</presentationURL>"
		"</device>"
		"</root>";

	res->println(buf);
}

void respondToSearch()
{
	String sIP = WebSocket::sIPAddr;

	String buf = "HTTP/1.1 200 OK\r\n"
		"EXT:\r\n"
		"CACHE-CONTROL: max-age=100\r\n" // SSDP_INTERVAL
		"LOCATION: http://" + sIP + ":80/description.xml\r\n"
		"SERVER: FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0\r\n" // _modelName, _modelNumber
		"hue-bridgeid: " + WebSocket::escapedMac + "\r\n"
		"ST: urn:schemas-upnp-org:device:basic:1\r\n"  // _deviceType
		"USN: uuid:2f402f80-da50-11e1-9b23-" + WebSocket::escapedMac + "::upnp:rootdevice\r\n" // _uuid::_deviceType
		"\r\n";

	espalexaUdp.beginPacket(espalexaUdp.remoteIP(), espalexaUdp.remotePort());
	espalexaUdp.write((uint8_t*)buf.c_str(), strlen(buf.c_str()));
	espalexaUdp.endPacket();
	LogLn("UDPResp: " + buf);
}

void handleAlexaApiCall(httpsserver::HTTPRequest* _req, httpsserver::HTTPResponse* res)
{
	LogLn("AlexaApiCall");
	String sResponse = "";

	std::string std_req = _req->getRequestString();
	String req(std_req.c_str());

	//get the content - and transfer buffer to a String
	size_t nBodyLen = _req->getContentLength();
	char cbody[nBodyLen + 1];
	_req->readChars(cbody, nBodyLen);
	cbody[nBodyLen] = 0;
	String body(cbody);
	

	if (body.indexOf("devicetype") > 0)
	{//client wants a hue api username - any name will do.
		LogLn("devicetype");
		body = "";
		sResponse = "[{\"success\":{\"username\":\"admin\"}}]";
	}
	else if ((req.indexOf("state") > 0) && (body.length() > 0))
	{//client wants to control light
		//respond quickly...
		sResponse = "[{\"success\":{\"/lights/1/state/\": true}}]";
		res->setHeader("Content-Type", "application/json");
		res->println(sResponse);
		res->finalize();

		LogLn("=HTTP REQUEST=");
		LogLn("Req: " + req);
		LogLn("Bdy: " + body);
		LogLn("Rsp: " + sResponse);
		LogLn("==============");

		uint32_t devId = req.substring(req.indexOf("lights") + 7).toInt();
		Logf("Light State: %d\n", devId);
		devId--; //zero-based for devices array
		if (devId > currentDeviceCount) {
			LogLn("*Error Incorrect Device %d*\n");
			return;
		}

		//]devices[devId]->setPropertyChanged(EspalexaDeviceProperty::none);

		if (body.indexOf("false") > 0) //OFF command
		{
			devValues[devId] = false;
			LogLn("**Update: " + devNames[devId] + " " + String(devValues[devId]));
		}
		else if (body.indexOf("true") > 0) //ON command
		{
			devValues[devId] = true;
			LogLn("**Update: " + devNames[devId] + " " + String(devValues[devId]));

			RKPClass::PushKey('0');
			RKPClass::PushKey('1');
			RKPClass::PushKey('1');
			RKPClass::PushKey('2');
			RKPClass::PushKey('2');

		}
		return;
	}
	else
	{
		//Request: GET /api/2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr/lights/1 
		//Request: GET /api/2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr/lights 
		//Request: GET /api/admin/lights/1 
		//int pos = req.indexOf("lights");
		int pNumber = 18;
		int pos = req.indexOf("/api/admin/lights");
		if (pos >= 0) //client wants light info
		{
			int devId = req.substring(pos + pNumber).toInt();
			if (devId == 0) //client wants all lights
			{
				LogLn("Light: All!");
				sResponse = "{";
				for (int i = 0; i < currentDeviceCount; i++)
				{
					sResponse += "\"" + String(i + 1) + "\":";
					sResponse += deviceJsonString(i + 1);;
					if (i < currentDeviceCount - 1)
						sResponse += ",";
				}
				sResponse += "}";
			}
			else //client wants one light (devId)
			{
				Logf("Light Char: %d\n", devId);
				if (devId > currentDeviceCount)
				{
					Serial.printf("*Error Incorrect Device %d*\n");
				}
				else
				{
					res->setHeader("Content-Type", "application/json");
					sResponse += deviceJsonString(devId);
				}
			}
		}
	}
	//we don't care about other api commands at this time and send empty JSON
	res->setHeader("Content-Type", "application/json");
	res->println(sResponse);
	res->finalize();

	Serial.println("===");
	Serial.println("Req: " + req);
	Serial.println("Bdy: " + body);
	Serial.println("Rsp: " + sResponse);
	Serial.println("===");
	return;
}

void AlexaStart(httpsserver::HTTPServer* secureServer)
{
	LogLn("Start Alexa");
	udpConnected = espalexaUdp.beginMulticast(IPAddress(239, 255, 255, 250), 1900);

	Logf("This hub is Hosting %d Devices\n", currentDeviceCount);
}

void AlexaLoop() {
	if (udpConnected)
	{
		int packetSize = espalexaUdp.parsePacket();
		if (packetSize < 1)
			return; //no new udp packet

		unsigned char packetBuffer[packetSize + 1]; //buffer to hold incoming udp packet
		espalexaUdp.read(packetBuffer, packetSize);
		packetBuffer[packetSize] = 0;

		espalexaUdp.flush();

		const char* request = (const char*)packetBuffer;

		//LogLn("Got UDP: ");
		//LogLn(request);
		//LogLn(";");

		if (strstr(request, "M-SEARCH") == nullptr)
			return;

		if (strstr(request, "ssdp:disc") != nullptr &&  //short for "ssdp:discover"
			(strstr(request, "upnp:rootd") != nullptr || //short for "upnp:rootdevice"
				strstr(request, "ssdp:all") != nullptr ||
				strstr(request, "asic:1") != nullptr)) //short for "device:basic:1"
		{
			LogLn("Responding search req...");
			respondToSearch();
		}
	}
}
