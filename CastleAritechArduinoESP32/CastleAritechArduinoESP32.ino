/*
*   Castle KeyPad
*
*   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more 
*   
*   For Arduino (UNO or Leonardo) with added Ethernet Shield
*   
*   See Circuit Diagram for wiring instructions
*
*   V1.04  March  2014: Arm Disarm Complete and tested as working
*   V1.2	  April  2014: Updated to use ABCD Keypad bus
*   V1.3	  April  2015: Updated to current Arduino IDE
*
*   Author: Ozmo
*
*   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
*
*
*
*  Compile for WEMOS LOLIN32 - a ESP32 module with a built in LCD display - you will need install the espressif\esp32 board support (download from espressif website)
*  
*  
*  Note: Warning displays - but correct Wifi Library was selected by ide....
*  Multiple libraries were found for "WiFi.h"
*     Used: C:\Projects\Arduino\TestProjects\hardware\espressif\esp32\libraries\WiFi
*     Not used: C:\Projects\ArduinoIDEs\arduino-1.8.5\libraries\WiFi
*
*/

//See Config.h for all the configuration you may need

#include "Config.h"
#include "Log.h"


#include "RKP.h"
#include "SMTP.h"
#include "WebSocket.h"


void setup()
{
  	//--debug logging start
  	Log_Init();

    LogLn(F("\r\n-----[Start]-----"));

    LogLn(F("RKP Init.."));
	  RKPClass::Init();
	
   	//--Create Web Server
    LogLn(F("Create WebSocket.."));
   	WebSocket::WebSocket_EtherInit();

    LogLn(F("SMTP Init.."));
   	SMTP::Init();

   	//SMTP::QueueEmail(START); //Creats a bootup email

	  //Flashing led on Pin 12 show us we are still working ok
    pinMode(ledFeedback,OUTPUT); digitalWrite(ledFeedback,LOW);
    //LogScreen("Loop Start..");
}

void loop()
{
  while(true)
  {
    static int tiLast = 0;
  	
    //Flash status led
  	int tiNow = millis();
  	if (tiLast < tiNow - 500)
  	{
  		tiLast = tiNow;
  		digitalWrite(ledFeedback, !digitalRead(ledFeedback));
  	}
  
    RKPClass::Poll();

    //Any browser activity? Set up new connections and deliver web pages
    WebSocket::EtherConnect();
    //Any browser activity?
    WebSocket::EtherPoll();

  	//Send Email if Alarm
  	//)if (SMTP::nEmailStage >= 0)
  	//)  SMTP::SendEmailProcess();
  }
}

