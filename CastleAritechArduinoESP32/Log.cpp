/*
 *
 * Created: 3/30/2014 11:35:06 PM
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


#include "Log.h"

#ifndef DEBUG_LOG
	void LogBuf(char* t){}
	void LogHex(byte rx){}
	void LogHex(char* s){}
	void LogHex(byte* s , int len){}
#else
	void LogBuf(char* t)
	{
		int ix=0;
		while(t[ix]!=0)
			Log((char)t[ix++]);
		LogLn("..");
	}
	void LogHex(byte rx)
	{//Show one two digit hex number
		if (rx<16) Log('0'); Log(rx,HEX);
		Log(' ');
	}
	void LogHex(char* s)
	{//Show a buffer as hex
		int n=0;
		while(s[n] != 0)
		{
			Log(s[n], HEX);
			Log(' ');
		}
		//LogLn("{end}");
	}
	void LogHex(byte* s , int len, boolean noNL)
	{
	  LogHex(s,len);
	}
	void LogHex(byte* s , int len)
	{
		const int l = 24;
		for(int col=0;;col++)
		{
			if ((col*l) >= len)		//len=10 = 1*8
				break;
			for(int r=0;r<l;r++)
			{
				if (r!=0) Log(' ');
				int x = col*l+r;
				//byte c = (x<len) ? s[col*l+r] : 0x00;
				//if (c<16) Log('0'); Log2(c, HEX);
				if (x<len)
				{
					byte c = s[col*l+r];
					if (c<16) Log('0');	Log(c, HEX); // displays 0 if necessary
				}
				else
					Log("--");
			}
			Log(':');
			for(int r=0;r<l;r++)
			{
				int x = col*l+r;
				byte c = (x<len) ? (s[col*l+r]) : '#'; ///strip bit 7 (flash)
                       
				if (c <= ' ' || c>= 0x7F)
				{Log('.');}		//brackets important
				else
				{Log((char)c);} //brackets important
			}
			LogLn("");
			//len-=l;
			//if (len<=0)
			//break;
		}
		//LogLn("{end}");
	}
#endif

void Log_Init()
{
#ifdef DEBUG_LOG
	Serial.begin(nSerialBaudDbg);
	delay(100); // Required to catch initial messages
	LogLn("Log Start");
	LCD_Init();
#endif
}



#ifdef ENABLE_DISPLAY
#include "oleddisplay.h"
#include "WebSocket.h" //for the panel display buffer

OLEDDisplay display(0x3c, 18, 19);  // was SSD1306Wire. ADDRESS, SDA, SCL

void LCD_Init()
{
	// Initialising the UI will init the display too.
	display.init();
	//display.flipScreenVertically(); //if required

	//display.setFont(ArialMT_Plain_10);
	//display.setTextAlignment(TEXT_ALIGN_LEFT);
	//display.setLogBuffer(5, 30);

	LogLn("Start");
}

//Send update to LCD only (not to WebSockets) 
//send to Websockets only if rkp screen changes
void FlagDisplayUpdate()
{
	bDisplayToSend = true;
}


void DisplayUpdateDo()
{
	display.clear();
	display.drawString(0, 0, "Wifi: " + String(gWifiStat));
	display.drawString(0, 10, "Clients: " + String(gClients));
	display.drawString(60, 10, "Status: " + (gPanelStat.length()==0? "Clear" : gPanelStat));
	display.drawString(0, 20, WebSocket::dispBufferLast);
	display.display();
}

#else

//this code for esp32s that dont have the built in lcd screen
void LCD_Init()
{
	LogLn("LCDInit Disabled");
}

void FlagDisplayUpdate()
{
}

void DisplayUpdateDo()
{
}

#endif

//Send to Display And to WebSocket
void FlagWebsocketUpdate()
{
	bWebSocketToSend = true;
	bDisplayToSend = true;   //we need send to LCD display also
}

//Store for what will be displayed next display update.
int gClients = 0;
String gWifiStat = "Off";
String gPanelStat = "";
bool bDisplayToSend = false;
bool bWebSocketToSend = false;

