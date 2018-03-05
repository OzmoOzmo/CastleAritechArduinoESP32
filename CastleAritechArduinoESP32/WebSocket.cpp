/*
 * WebSocket.cpp - Websocket Implementation - works on most browsers and Mobile Phones (but notably Not on IE8)
 *
 * Created: 3/30/2014 9:57:39 PM
 *
 *   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more
 *
 *   For Arduino (UNO or Leonardo) with added Ethernet Shield
 *
 *   See Circuit Diagram for wiring instructions
 *
 *   Author: Ozmo
 *
 *   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
 *
*/

#include "LOG.h"
#include <WiFi.h> //found here: "\Documents\Arduino\hardware\espressif\esp32\libraries\WiFi.h"
#include "websocket.h"
#include "sha1.h"
#include "RKP.h"  //for nKeyToSend

/*
#ifdef AtmelStudio
  //Other modules
  #include <SPI.cpp>
  #include <Ethernet.cpp>
  #include <EthernetClient.cpp>
  #include <EthernetServer.cpp>
  #include <utility\socket.cpp>
  #include <utility\w5100.cpp>
#else
  #include <SPI.h>
  #include <Ethernet.h>
  #include <EthernetClient.h>
  #include <EthernetServer.h>
  //#include <util.h>
#endif
*/

//Wifi Password //TODO: Move to config
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

WiFiServer ethServer(IP_P);
WiFiClient ethClient;

//"*" will be replaced with button
/*const char htmlSite[] PROGMEM=*/
const char* htmlSite= //TODO: move to flash?
//"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN'>"
"<!DOCTYPE html>"
"<html><head><title>Castle</title>"
"<meta name='viewport' content='width=320, initial-scale=1.2, user-scalable=no'>" //"no" because pressing buttons causes screen to unzoom
"<style>.long{height: 64px;} button{height: 35px;width: 35px;}</style>"
"<script src='http://goo.gl/m3GB3M' type='text/javascript'></script>"
"</head><body>"
"<div style='border: 5px solid black; width: 180px;'>&nbsp;<div id=msg1 style='float:left'></div><div id=msg2 style='float:right'></div></div>"
"<table>"
"<tr><td><button>1</button></td><td><button>2</button></td><td><button>3</button></td><td rowspan=2><button class=long>Y</button></td></tr>"
"<tr><td><button>4</button></td><td><button>5</button></td><td><button>6</button></td></tr>"
"<tr><td><button>7</button></td><td><button>8</button></td><td><button>9</button></td><td rowspan=2><button class=long>N</button></td></tr>"
"<tr><td><button>*</button></td><td><button>0</button></td><td><button>#</button></td></tr>"
"</table>"
"<script>var ws;$(document).ready(function(){"
"try{"
  "ws = new WebSocket('ws://'+location.hostname+':" STR(IP_P) "/sock/');"
  "ws.onmessage = function (evt) {var d=evt.data.split('|');$('#msg1').text(d[0]);$('#msg2').text(d[1]);};"
  "ws.onerror = function (evt) {$('#msg').append('ERR:' + evt.data);};"
  "ws.onclose = function (evt) {$('#msg').text('Closed');};"
  "ws.onopen = function () { };"
"} catch (ex) {alert(ex.message);}"
"$(document).keypress(function (e) {ws.send(e.which);});"
"$(':button').click(function (e) { ws.send(e.target.innerText.charCodeAt(0));});"
"});</script></body></html>";

// Create a Websocket server
void WebSocket::WebSocket_EtherInit()
{
    LogScreen("Connecting to WIFI");
    //LogScreen(ssid);

    IPAddress ip(IP_A, IP_B, IP_C, IP_D);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip,ip,subnet);
    WiFi.begin(ssid, password);

/* Interesting more direct way...

    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "access_point_name",
            .password = "password",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;
    while (true) {
        gpio_set_level(GPIO_NUM_4, level);
        level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
 
 */

  //LogLn(Ethernet.localIP());
}

bool WebSocket::bConnected = false;

//here every 500ms or so - check if connected
void WebSocket::EtherConnect()
{
  if (!bConnected)
  {
    if (WiFi.status() != WL_CONNECTED)
        return;

    //LogScreen("WiFi connected.");
    LogScreen("IP:" + WiFi.localIP().toString() + ":" + IP_P);
    ethServer.begin();
    bConnected = true;
  }
}

void WebSocket::EtherPoll()
{
  if (!bConnected)
    return;

  // Should be called for each loop.
  WiFiClient cli=ethServer.available();

  /* this section is UNDER CONSTRUCTION!!!
  if (cli)
  {
    if (ethClient == NULL) //]if (ethClient != cli)
    {//New connection
      //LogLn(F("new"));
      //Secrisk
      ethClient = cli;
      WebSocket_doHandshake();
    }
    else
    {//Existing connection
      LogLn(F("existing"));
      if (WebSocket_getFrame() == false)
      {//Request to end comms (rarely happens)
        //Got bad frame, disconnect
        Log(F("Disconnect{"));
        while(ethClient.available()>0)
          ethClient.read();
        ethClient.flush();
        ethClient.stop();
        ethClient = NULL;
      }
    }
  }
  */

  
}

void WebSocket::WebSocket_doHandshake()
{
  LogLn(F("Do Handshake"));

  bool hasKey = false;
  bool bReqWebPage = false;

  byte counter = 0;

  while(ethClient.available()>0) //TODO: move this to state machine
  {//Read each line
    //TODO: ignore any lines not starting with Sec-WebSocket-Key: if (counter.length==18){}

    byte bite = ethClient.read();
    //Log((char)bite); LogLn("");
    htmlline[counter++] = bite;
    if (counter > (127))
    {
      Log(F("Ignoring Oversized Line{\n  "));
      LogHex((byte*)htmlline, counter-1);
      //while(ethClient.available()>0) //TODO: issue with original code - shouldnt dump this
      //  Log((char)ethClient.read());
      //  //LogHex(ethClient.read());
      LogLn(F("}"));
      counter=0;
      continue;
    }

    if (bite == '\n')
    { // Parse the line
      htmlline[counter - 2] = 0; // Terminate string before CRLF

      bool bFound = (strstr_P(htmlline, PSTR("Sec-WebSocket-Key:")) != NULL);
      if (bFound)
      {
        Log("Key>");LogLn(htmlline);
        hasKey = true;
        strtok(htmlline, " ");
        strcpy(key, strtok(NULL, " ")); //TODO: Not safe - need specify max 80 limit
      }

      if (strstr_P(htmlline, PSTR("GET / HTTP")) != NULL)
      {
        LogLn(F("Page Req"));
        bReqWebPage=true;
      }

      counter = 0; // Start saving new header string
    }
  }

  //Log("Got header: ");LogHex((byte*)htmlline,counter);

  if (hasKey)
  {
    LogLn(F("WS Req"));
    strcat(key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"); //TODO: simplify /*strcat_P(key, PSTR("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")); // Magic Number GUID*/
    
    Sha1.init();
    Sha1.print(key);
    uint8_t* hash = Sha1.result();
    base64_encode(htmlline, (char*)hash, 20); //reuse htmlline buffer
    ethClient.print(F("HTTP/1.1 101 Switching Protocols\r\n"));
    ethClient.print(F("Upgrade: websocket\r\n"));
    ethClient.print(F("Connection: Upgrade\r\n"));
    ethClient.print(F("Sec-WebSocket-Accept: "));
    ethClient.print(htmlline);  //eg. VoNhf1LMVVTziHWxjiajVem5DB4=
    ethClient.print(F("\r\n\r\n"));

    LogLn(F("Connected"));

    //Send any display we might have
    RKPClass::SendDisplayToWebClient(); //RKPClass::bScreenHasUpdated = true; //RKPClass::SendDisplayToBrowser();
  }
  else if (bReqWebPage)
  {// Nope, Not a websocket request - send the main webpage
    //LogLn(F("WebPageReq"));
    SendHTMLSite();
  }
  else
  {
    ethClient.println(F("HTTP/1.0 404 File Not Found"));  //
    ethClient.println(F("Content-Type: text/html"));
    ethClient.println(F("Connnection: close"));
    ethClient.println();
    //delay(1);
    ethClient.stop();
  }
  return;
}

void WebSocket::SendHTMLSite()
{
  //Log(F("ConnectWWW{"));
  //  while(ethClient.available()>0)
  //  Log((char)cli.read());
  //LogLn(F("}"));
  ethClient.println(F("HTTP/1.0 200 OK"));  //
  ethClient.println(F("Content-Type: text/html"));
  ethClient.println(F("Connnection: close"));
  ethClient.println();
  /*const char* p = &htmlSite[0];
  for(int n=0;n<2000;n++)
  {
    char c = pgm_read_byte(p++);
    if (c==0) break;
    //Can use tokens to compress the HTML a bit
    //if (c=='*')
    //  ethClient.print(F("button"));
    //else
    ethClient.print(c);
  }*/
  ethClient.print(htmlSite);

  // close the connection so browser gets data
  ethClient.flush();
  //delay(1);
  ethClient.stop();
  LogLn("Sent Page");
}

/*//see if any web clients are calling to connect
//void WebSocket::EtherPoll()
//{
//  if (!bConnected)
//        return;
// 
//  WiFiClient client = server.available();   // listen for incoming clients
//
//  if (client) {                             // if you get a client,
//    Serial.println("New Client.");           // print a message out the serial port
//    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(5, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}*/


//Send something to connected browser
bool WebSocket::WebSocket_send(char* data, byte length)
{
  LogLn("Sending Screen");LogHex((byte*)data,length);
  
  if (!ethClient.connected())
  {
    LogLn(F("No Client."));
    return false;
  }
  //int length = strlen(data);
  ethClient.write(0x81);// string type

  if (length > 125) {
    ethClient.write(126);
    ethClient.write((uint8_t) (length >> 8));
    ethClient.write((uint8_t) (length && 0xFF));
  }
  else
    ethClient.write((uint8_t) length);

  for (int i=0; i<length; ++i)
    ethClient.write(data[i]);

  LogLn(F("Sent OK."));
    
  return true;
}


/*

//--Sockets
//use this as password - pick random port - set code tamper on alarm also.
EthernetServer ethServer = EthernetServer(IP_P);


// Create a Websocket server
void WebSocket::WebSocket_EtherInit()
{
	IPAddress ip( IP_A, IP_B, IP_C, IP_D);	    //Give the device a unique IP
	IPAddress gateway( IP_A, IP_B, IP_C, 1 );   //Gateway (youre Router)
	IPAddress subnet( 255, 255, 255, 0 );	    //typically dont need change

	// this sequence must be unique on your lan
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 };	//typically dont need change

	//Start Ethernet
	Ethernet.begin(mac, ip);

	ethServer.begin();
        delay(1000); // Settle time
	Log(F("server is at ")); LogLn(Ethernet.localIP());
}
*/

//used by base64
inline void WebSocket::a3_to_a4(unsigned char * a4, unsigned char * a3) {
	a4[0] = (a3[0] & 0xfc) >> 2;
	a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
	a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
	a4[3] = (a3[2] & 0x3f);
}

int WebSocket::base64_encode(char *output, char *input, int inputLen)
{
	int i = 0, j = 0;
	int encLen = 0;
	unsigned char a3[3];
	unsigned char a4[4];

        static char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	while(inputLen--) {
		a3[i++] = *(input++);
		if(i == 3) {
			a3_to_a4(a4, a3);

			for(i = 0; i < 4; i++) {
				output[encLen++] = b64_alphabet[a4[i]];
			}

			i = 0;
		}
	}

	if(i) {
		for(j = i; j < 3; j++) {
			a3[j] = '\0';
		}

		a3_to_a4(a4, a3);

		for(j = 0; j < i + 1; j++) {
			output[encLen++] = b64_alphabet[a4[j]];
		}

		while((i++ < 3)) {
			output[encLen++] = '=';
		}
	}
	output[encLen] = '\0';
	return encLen;
}


char WebSocket::htmlline[128];	//There are 3 buffers needed - htmlline, key and sha - sha and htmlline share the same temp buffer
char WebSocket::key[80];        //this cannot use htmlline also


struct Frame {
	bool isMasked;
	bool isFinal;
	byte opcode;
	byte mask[4];
	byte length;
	char data[64+1];
} frame;

byte WebSocket::ReadNext()
{
	byte bite = ethClient.read();
	//LogHex(bite);
	return bite;
}

bool WebSocket::WebSocket_getFrame()
{
	// Get opcode
	byte bite = ReadNext(); if (bite==0xFF) return false;

	frame.opcode = bite & 0xf; // Opcode
	frame.isFinal = bite & 0x80; // Final frame?
	// Determine length (only accept <= 64 for now)
	bite = ReadNext(); if (bite==0xFF) return false;

	frame.length = bite & 0x7f; // Length of payload
	if (frame.length >= 64)
	{//Unlikely to happen
		Log(F("Frame Too Big")); LogLn(bite);
		return false;
	}
	// Client should always send mask, but check just to be sure
	frame.isMasked = bite & 0x80;
	if (frame.isMasked) {
		frame.mask[0] = ReadNext();
		frame.mask[1] = ReadNext();
		frame.mask[2] = ReadNext();
		frame.mask[3] = ReadNext();
	}

	// Get message bytes and unmask them if necessary
	int i = 0;
	for (; i < frame.length; i++)
	{
		bite = ReadNext();
		if (frame.isMasked)
			frame.data[i] = bite ^ frame.mask[i % 4];
		else
			frame.data[i] = bite;
	}
	frame.data[i]=0;

	// Frame complete!
	if (!frame.isFinal)
	{	// We don't handle fragments! Close and disconnect.
		LogLn(F("Unsurp"));
		return false;
	}

	if (frame.opcode== 0x01)
	{// Txt frame
		Log(F("Data: ")); //Got Data
		LogLn(frame.data);
		RKPClass::PushKey(atoi(frame.data));
		return true;
	}

	if (frame.opcode== 0x08)
	{
		// Close frame. Answer with close and terminate tcp connection
		LogLn(F("Close")); //Close frame
		ethClient.write((uint8_t) 0x08);
		return false;
	}

	// Unexpected. Ignore?
	LogLn(F("Ex.")); //Unhandled frame ignored
	return false;
}


