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
*  Compile for ESP32 Dev Module 
*  For best results : Use on a "D1 R32" WEMOS LOLIN32 - a ESP32 module with a built in LCD display 
*  
*  You will need install the espressif\esp32 board support from within Arduino IDE
*
*
* You will also need install:
*   esp32_https_server
*     https://github.com/fhessel/esp32_https_server  - for https, wss sockets
*     
*   esp8266-oled-ssd1306
*     https://github.com/ThingPulse/esp8266-oled-ssd1306 - for LCD Support- works well on ESP32
*
*/

//See Config.h for all the configuration you may need

#include "Log.h"
#include "RKP.h"
#include "SMTP.h"
#include "WebSocket.h"
#ifdef VM
  #include "libs\Preferences\Preferences.h" 
#else
  #include <Preferences.h>
#endif

Preferences prefs;

void setup()
{
    //--debug logging start
    Log_Init();

    LogLn("\r\n-----[Start]-----");

    //NVM Init
    int result = prefs.begin("castle", false);
    Logf("PrefInit: %u\n", result);
    unsigned int counter = prefs.getUInt("counter", 0)+1;
    Logf("Reboot counter: %u\n", counter);
    prefs.putUInt("counter", counter);

    LogLn("Display Init..");
    LCD_Init();

    LogLn("RKP Init..");
    RKPClass::Init();

    LogLn("Server Init..");
    WebSocket::ServerInit();

    LogLn("SMTP Init..");
    SMTP::QueueEmail(MSG_START); //Creates a bootup email

    SMTP::StartEmailMonitor();

    WebSocket::StartWebServerMonitor();

    //Flashing led on Pin 12 show us we are still working ok
    LogLn(F("If LED not Blinking - Ensure Panel is Connected."));
    pinMode(ledFeedback, OUTPUT); digitalWrite(ledFeedback, LOW);

    //Terrible performance for RKP when in a thread - we will run on core0

    #ifndef DEBUG_LOG
        Serial.print("(Mostly)Silent Mode enabled.");
    #else
        Serial.print("Enable Silent Mode for full speed.");
    #endif
}

void loop()
{
    LogLn("Main Loop");
    while (true)
    {
        RKPClass::Poll();

        //Flash status led
        static int tiLast = 0;
        int tiNow = millis();
        if (tiLast < tiNow - 500){
            tiLast = tiNow;
            digitalWrite(ledFeedback, !digitalRead(ledFeedback));
        }
    }
}

