/*
 * Created: 3/30/2014 11:35:06 PM
 *
 *   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more
 *
 *   For Arduino (UNO or Leonardo) with added Ethernet Shield
 *
 *   See Circuit Diagram for wiring instructions
 *
 *   Author: Ambrose Clarke
 *
 *   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
 *
*/

#ifndef LOG_h
#define LOG_h

#include "Config.h"

#include <Arduino.h>

#ifdef DEBUG_LOG

  #define nSerialBaudDbg 115200
  
  //this makes it easy to remove from release for max speed
  #define Log Serial.print
  #define LogLn Serial.println
  #define Logf Serial.printf
    
#else
	//#define Log(x) {}
	//#define Log2(x,y) {}
	//#define LogLn(x) {}
	//#define LogLn2(x,y) {}
  
  //this makes it easy to remove from release for max speed
  #define Log //
  #define LogLn //
  #define Logf //
  
#endif


void LogHex(byte rx);
void LogHex(char* s);
void LogHex(byte* s , int len);

void Log_Init();

//Built in LCD 
#define DISPLAY_WIDTH 128
void LCD_Init();

extern int gClients; 
extern String gWifiStat;
extern String gPanelStat;
extern bool bDisplayToSend;
extern bool bWebSocketToSend;

void FlagDisplayUpdate();
void FlagWebsocketUpdate();
void DisplayUpdateDo();

#endif
