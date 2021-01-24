/*
 * WebSocket.cpp - Websocket Implementation - works on most modern browsers and Mobile Phones
 *
 * Created: 3/30/2014 9:57:39 PM
 *
 *   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more
 *
 *   For ESP32
 *
 *   See Circuit Diagram for wiring instructions
 *
 *   Author: Ambrose Clarke
 *
 *   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
 *
*/

#define WEBSERVER
#define WEBSOCKET


#include "LOG.h"
#ifdef VM
  #include "libs\WiFi\WiFi.h"
  #include "libs\Preferences\Preferences.h"
#else
  #include <WiFi.h>
  #include <Preferences.h>
#endif

#include "WebSocket.h"
#include "RKP.h"  //for PushKey
#include <sstream>

int WebSocket::nConnectState = WIFI_DOWN; //0 = no wifi  1= wificonnecting 2=wifi+sockets ok
char WebSocket::dispBufferLast[DISP_BUF_LEN + 1] = "RKP Unitialised";

#ifdef WEBSERVER
#ifdef VM
  #include "libs\WiFi\WiFi.h"
  #include "libs\ESPmDNS\ESPmDNS.h"
  #include "libs\HttpsServer\HTTPRequest.hpp"
  #include "libs\HttpsServer\HTTPResponse.hpp"
  #include "libs\HttpsServer\WebsocketHandler.hpp"
  #include "libs\HttpsServer\WebsocketNode.hpp"
#else
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <HTTPRequest.hpp>
  #include <HTTPResponse.hpp>
  #include <WebsocketHandler.hpp>
  #include <WebsocketNode.hpp>
#endif

#ifdef HTTPS
  #ifdef VM
    #include "libs\HttpsServer\HTTPSServer.hpp"
    #include "libs\HttpsServer\SSLCert.hpp"
  #else
    #include <HTTPSServer.hpp>
    #include <SSLCert.hpp>
  #endif
  
  httpsserver::SSLCert* cert; //our server certificate
  httpsserver::HTTPSServer* secureServer; //the server
#else
  #ifdef VM
    #include "libs\HttpsServer\HTTPServer.hpp"
  #else
    #include <HTTPServer.hpp>
  #endif
  httpsserver::HTTPServer* secureServer; //the server
#endif

extern Preferences prefs;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef HTTPS
#define WS "wss"
#else
#define WS "ws"
#endif

const String htmlSite =
"<!DOCTYPE html>"
"<html><head><title>Castle</title>"
"<meta name='viewport' content='width=320, initial-scale=1.8, user-scalable=no'>" //"no" as screen unzooms when press button
"<style>.long{height: 64px;} button{height: 35px;width: 35px;}</style>"
"</head><body>"
"<div style='border: 5px solid black; width: 180px;'>&nbsp;<div id=msg1 style='float:left'>%</div><div id=msg2 style='float:right'>%</div></div>"
"<table id='table'>"
"<tr><td><button>1</button></td><td><button>2</button></td><td><button>3</button></td><td rowspan=2><button class=long>Y</button></td></tr>"
"<tr><td><button>4</button></td><td><button>5</button></td><td><button>6</button></td></tr>"
"<tr><td><button>7</button></td><td><button>8</button></td><td><button>9</button></td><td rowspan=2><button class=long>N</button></td></tr>"
"<tr><td><button value='*'>" KEY_STAR "</button></td><td><button>0</button></td><td><button value='#'>" KEY_POUND "</button></td></tr>"
"</table>"
"<script defer>var ws;"
"function ge(x){return document.getElementById(x);}\n"
"function st(x,y){ge('msg1').innerText=x;ge('msg2').innerText=y?y:' ';}\n"
"try{"
"ws = new WebSocket('" WS "://'+location.hostname+'/sock');"
"ws.onmessage = function(evt){var d=evt.data.split('|');st(d[0],d[1]);}\n"
"ws.onerror = function(evt){st('ERR:' + evt.data,'');}\n"
"ws.onclose = function(evt){st('Connection Closed','');}\n"
"ws.onopen = function(){ws.send('r');}\n"
//pc keyboard support
"document.body.onkeydown = function(e){ws.send(String.fromCharCode(e.keyCode));}\n"
//buttons on html
"ge('table').onclick = function(e){ws.send(e.target.value || e.target.innerText);}\n"
"} catch(ex) {alert(ex.message);}\n"
"</script></body></html>";

// As websockets are more complex, they need a custom class that is derived from WebsocketHandler
class WebSockHandler : public httpsserver::WebsocketHandler {
public:
	static WebsocketHandler* create();
	void onMessage(httpsserver::WebsocketInputStreambuf* input);
	//]void initialize(httpsserver::ConnectionContext* con);
	void onClose();
};
// Max clients to be connected to the chat
WebSockHandler* activeClients[MAX_CLIENTS];


httpsserver::WebsocketHandler* WebSockHandler::create() {
	//Send welcome message handler->send(dispBufferLast, SEND_TYPE_TEXT);
	WebSockHandler* handler = new WebSockHandler();
	int i=0;
	for (; i < MAX_CLIENTS; i++) {
		if (activeClients[i] == nullptr) {
			activeClients[i] = handler;
			Logf("New client! :%d\n", i);
			break;
		}
	}
	if (i == MAX_CLIENTS)
	{
		Logf("Too many Clients!:%d\n", i);
	}

	//quick count...
	gClients = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
		if (activeClients[i] != nullptr)
			gClients++;
	FlagDisplayUpdate();
	return handler;
}

// When the websocket is closing, we remove the client from the array
void WebSockHandler::onClose() {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (activeClients[i] == this) {
			activeClients[i] = nullptr;
			Logf("Close client :%d",i);
		}
		else if (activeClients[i] != nullptr) //else used because one closing will be null 
			gClients++;
	}
	//quick count...
	gClients = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
		if (activeClients[i] != nullptr)
			gClients++;
	FlagDisplayUpdate();
}

// send s to all other clients
void WebSockHandler::onMessage(httpsserver::WebsocketInputStreambuf* inbuf) {
	// Get the input message
	std::ostringstream ss;
	ss << inbuf;
	std::string payload = ss.str();
	LogLn(payload.c_str());
	RKPClass::PushKey(payload.c_str()[0]);
}


//initialise what we can before wifi starts
void WebSocket::ServerInit()
{
	#ifdef HTTPS
	const char* PKName = "PK";
	const char* CertName = "CERT";
	const char* dn = "CN=castle.local,O=castle,C=IE";
	size_t pkLen = prefs.getBytesLength(PKName);
	size_t certLen = prefs.getBytesLength(CertName);
	if (pkLen && certLen)
	{
		LogLn("Found Cert In NVM");
		//create in heap
		uint8_t* pkBuffer = new uint8_t[pkLen];
		prefs.getBytes(PKName, pkBuffer, pkLen);
		uint8_t* certBuffer = new uint8_t[certLen];
		prefs.getBytes(CertName, certBuffer, certLen);

		cert = new httpsserver::SSLCert(certBuffer, certLen, pkBuffer, pkLen);
		#if DUMP_KEYS
		Serial.println("Retrieved Private Key " + String(cert->getPKLength()));
		for (int i = 0; i < cert->getPKLength(); i++)
			Serial.print(cert->getPKData()[i], HEX);
		Serial.println();

		Serial.println("Retrieved Certificate " + String(cert->getCertLength()));
		for (int i = 0; i < cert->getCertLength(); i++)
			Serial.print(cert->getCertData()[i], HEX);
		Serial.println();
		#endif
		LogLn("Cert Loaded");
	}
	else
	{
		LogLn("Generating certificate");
		cert = new httpsserver::SSLCert();
		int result = httpsserver::createSelfSignedCert(*cert, httpsserver::KEYSIZE_1024, dn);
		if (result != 0)
			LogLn("Error :(");
		else{
			LogLn("Generated OK");
			prefs.putBytes(PKName, (uint8_t*)cert->getPKData(), cert->getPKLength());
			prefs.putBytes(CertName, (uint8_t*)cert->getCertData(), cert->getCertLength());
			LogLn("Cert Stored");
		}
	}
	
	secureServer = new httpsserver::HTTPSServer(cert);
	#else
	secureServer = new httpsserver::HTTPServer();
	#endif

	httpsserver::ResourceNode* nodeRoot = new httpsserver::ResourceNode("/", "GET",
		[](httpsserver::HTTPRequest* req, httpsserver::HTTPResponse* res)
		{//root hander
			String ip = req->getClientIP().toString();
			std::string sReq = req->getRequestString();
			Log("[" + ip + "] GET root");

			//this is more memory efficient - writes, replacing % with the current rkp display
			int i1 = htmlSite.indexOf('%');
			int i2 = htmlSite.indexOf('%', i1 + 1);
			const char* pStr = htmlSite.c_str();
			res->write((byte*)pStr, i1); //first part
			res->write((byte*)dispBufferLast,16); //first 15 characters - everything up to |
			res->write((byte*)pStr+i1+1, i2-i1-1); //second part
			res->print(dispBufferLast+17); //alarm etc
			res->print(pStr + i2 + 1); //third part to end
			res->finalize();
		});
	secureServer->registerNode(nodeRoot);
	
	httpsserver::ResourceNode* nodeFavicon = new httpsserver::ResourceNode("/favicon.ico", "GET",
	[](httpsserver::HTTPRequest* req, httpsserver::HTTPResponse* res) {
			// Set Content-Type
			res->setHeader("Content-Type", "image/vnd.microsoft.icon");
			// Binary data for the favicon
			byte FAVICON_DATA[] = 
			{
				0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x08, 0x09, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x78, 0x00,
				0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x12, 0x00,
				0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0xc1, 0x1e,
				0x00, 0x00, 0xc1, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x9c, 0x00, 0x00, 0x00, 0x9c, 0x00, 0x00, 0x00, 0x88, 0x00,
				0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xa2, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0xa2, 0x00,
				0x00, 0x00, 0xeb, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
		
			#define FAVICON_LENGTH (sizeof(FAVICON_DATA)/sizeof(FAVICON_DATA[0]))
			
			res->write(FAVICON_DATA, FAVICON_LENGTH);
		}
	);
	secureServer->registerNode(nodeFavicon);
	
	
	// Add sock node like folder node
	httpsserver::WebsocketNode* websockNode = new httpsserver::WebsocketNode("/sock", &WebSockHandler::create);
	secureServer->registerNode(websockNode);
}
#endif

//this is called constantly to service network requests
void WebSocket::EtherPoll()
{
	if (nConnectState < WIFI_OK)
		//No web server just yet
		return;

	//TODO: need test if this is enough to reset wifi connection if lost...
	if (nConnectState != WIFI_PENDING && WiFi.status() != WL_CONNECTED) {
		LogLn("EtherPoll-Wifi reconnect");
		nConnectState = WIFI_DOWN; //switch to retry connecting to wifi again...
		return;
	}

#ifdef WEBSERVER
	secureServer->loop();//service webserver
#endif
}


void WebSocket::StartWebServerMonitor()
{
	xTaskCreatePinnedToCore(
		[](void* parameter) {
			while (true)
			{
				//Serial.println("Test Wifi");
				WebSocket::Verify_WiFi();
				delay(500);
				#ifdef REPORT_STACK_SPACE
				Logf("threadWIFI %d\n", uxTaskGetStackHighWaterMark(NULL));
				#endif
			}
		},
		"threadWIFI",     // Task name
		5000,            // Stack size (bytes)
		NULL,             // Parameter
		1,                // Task priority
		NULL,             // Task handle
		1                 //ARDUINO_RUNNING_CORE
	);

	xTaskCreatePinnedToCore(
		[](void* parameter)
		{
			while (true)
			{
				WebSocket::EtherPoll();
				delay(75);

				#ifdef REPORT_STACK_SPACE
				static int n = 0;
				if (n++ % 10 == 0)
					Logf("threadHTTPS %d\n", uxTaskGetStackHighWaterMark(NULL));
				#endif
			}
		},
		"threadHTTPS",  // Task name
		10000,            // Stack size (bytes)
		NULL,             // Parameter
		1,                // Task priority
		NULL,             // Task handle
		1                 //ARDUINO_RUNNING_CORE
	);

	//Do all LCD updates in Wifi/WebServer/UI Thread
	xTaskCreatePinnedToCore(
		[](void* parameter)
		{
			while (true)
			{
				if (bDisplayToSend)
				{
					bDisplayToSend = false;
					DisplayUpdateDo();
				}

				if (bWebSocketToSend)
				{
					bWebSocketToSend = false;
					WebSocket::WebSocket_send();
				}

				delay(10);

				#ifdef REPORT_STACK_SPACE
				static int n=0;
				if (n++ % 10 == 0)
					Logf("threadLCD %d\n", uxTaskGetStackHighWaterMark(NULL));
				#endif
			}
		},
		"threadLCD",  // Task name
			3000,            // Stack size (bytes)
			NULL,             // Parameter
			1,                // Task priority
			NULL,             // Task handle
			1                 //ARDUINO_RUNNING_CORE
		);
}

//Send something to connected browser
bool WebSocket::WebSocket_send()
{
	const char* data = WebSocket::dispBufferLast;

	// Send it back to every client
	for (int i = 0; i < MAX_CLIENTS; i++) {
		WebSockHandler* pClient = activeClients[i];
		if (pClient != nullptr) {
			pClient->send(data, httpsserver::WebsocketHandler::SEND_TYPE_TEXT);
			//odd without next line - the compiler sends to only 1 handler... compiler bug?
			Logf("Screen Sent to:%d\n",i);
		}
	}
	//Very useful: LogHex((byte*)data,length);
	return true;
}

// Start process to join a Wifi connection
void WebSocket::WebSocket_WiFi_Init()
{
	//Wifi Password is defined in config.h
	//gWifiStat = "Connecting: " WIFI_SSID; DisplayUpdate(); //ToScreen(0, "Connecting: " WIFI_SSID);

	IPAddress ip_me; ip_me.fromString(IP_ME);
	IPAddress ip_gw; ip_gw.fromString(IP_GW);
	IPAddress ip_sn; ip_sn.fromString(IP_SN);
	IPAddress ip_dns; ip_dns.fromString(IP_DNS);
	
	WiFi.config(ip_me, ip_gw, ip_sn, ip_dns, ip_dns);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	nConnectState = WIFI_PENDING;
	LogLn("End EtherInit");
}

//here every 500ms or so - check if connected
void WebSocket::Verify_WiFi()
{
	static int nErrorCount = 0;
	if (nConnectState == WIFI_DOWN)
	{
		LogLn(F("Start Wifi"));
		gWifiStat = "Cnt: " WIFI_SSID; FlagDisplayUpdate(); //ToScreen(0, "Connecting: " WIFI_SSID);
		WebSocket::WebSocket_WiFi_Init();
		nConnectState = WIFI_PENDING;
		nErrorCount = 0;
		return;
	}

	if (nConnectState == WIFI_PENDING)
		if (WiFi.status() != WL_CONNECTED)
		{
			if (nErrorCount++ < 10){
				Log(F("Connecting.."));
			}
			else
				nConnectState = WIFI_DOWN;
			return;
		}
		else
		{//just got good wifi - set up socket server & web server
			LogLn("WiFi connected.");
			//LogScreen("IP:" + WiFi.localIP().toString() + ":" + IP_P);
			//ToScreen(0, "IP:" + WiFi.localIP().toString() + ":" + IP_P);
			gWifiStat = "IP:" + WiFi.localIP().toString() + ":" + IP_P; FlagDisplayUpdate();
#ifdef WEBSERVER
			secureServer->start();
			if (secureServer->isRunning())
				Serial.println("Server ready.");
			//start_mdns_service();
#endif

			nConnectState = WIFI_OK;
			return;
		}

	if (WiFi.status() != WL_CONNECTED)
	{
		//failed to connect - try again
		LogLn(F("Retry connect Wifi"));
		nConnectState = WIFI_DOWN;
		return;
	}
}
