/*
 * Log.cpp - Logs all to Hyterterminal or other via Arduino Pin 11
 *
 * 'Serial1'(Pin1&2) is used for Panel TX and the USB 'Serial' is really bad for debugging on Leonardo - so we use Pin 11.
 *
 * Note: Have QUIET Set during Normal use - some panels require full speed
 *
 * Created: 3/30/2014 11:35:06 PM
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


#include "Log.h"


#ifdef QUIET
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
	//LogLn(".");
	LogLn("..");
}
void LogHex(byte rx)
{//Show one two digit hex number
	if (rx<16) Log('0');
	Log2(rx,HEX);
	Log(' ');
}
void LogHex(char* s)
{//Show a buffer as hex
	int n=0;
	while(s[n] != 0)
	{
		Log2(s[n], HEX);
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
				if (c<16) Log('0'); Log2(c, HEX);
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


//Using \libraries\esp8266-oled-ssd1306
// Initialize the OLED display using Wire library
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"
SSD1306Wire display(0x3c, 5, 4);

void Log_Init()
{
#ifndef QUIET
	Serial.begin(nSerialBaudDbg);
  LogLn("Log Start");
#endif

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically(); //if required
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.setLogBuffer(5, 30);
  LogScreen("Start");
}

//Clears a line on the screen and puts the text to that line
void ToScreen(int line, String text)
{//see also \libraries\esp8266-oled-ssd1306

  //Clearline
  /*const int CHAR_HEIGHT = 11;
  for(int y=line;y<(line+CHAR_HEIGHT);y++)
  {
    int length = DISPLAY_WIDTH;
    uint8_t* bufferPtr = display.buffer + ((y >> 3) * DISPLAY_WIDTH);
    uint8_t drawBit = 1 << (y & 7);
    drawBit = ~drawBit;
    while (length--)
        *bufferPtr++ &= drawBit;
  }*/
  //top 8 lines
  memset(display.buffer, 0, DISPLAY_WIDTH);
  //next 4 lines
  uint8_t* bufferPtr = display.buffer + DISPLAY_WIDTH;
  uint8_t drawBit = 0xF0;
  int length = DISPLAY_WIDTH;
  while (length--){
    *bufferPtr &= drawBit;
    *bufferPtr++ |= 0x40;
  }
      
  display.drawString(0, line*10, text);
  display.display();
}

void LogScreen(String text)
{
    //display.clear();
    display.println(text); // Print to the screen
    display.drawLogBuffer(0, 16); // Draw it to the internal screen buffer
    
    display.display(); // Display it on the screen
    LogLn(text);
}


