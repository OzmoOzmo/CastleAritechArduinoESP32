/*
 * RKP.h
 *
 * Created: 3/30/2014 11:18:18 PM
 */ 

#ifndef RKP_h
#define RKP_h

#include "Arduino.h"

#define maxkeybufsize 8
#define DISP_BUF_LEN 16+1+2		//16 characters - space - AW (will be followed by a Terminator 0)

class FIFO
{
	public:
		FIFO();
		byte pop();
		void push( byte b );
	private:
		int nextIn;
		int nextOut;
		int count;
		static byte raw[maxkeybufsize];
};


class RKPClass
{
 private:
		static FIFO fifo;

    bool static loop_PanelMon();
    
    static void SerialInit();
 
    
public:
    void static Poll();

    
    //TODO: move private
		void static SendItems();
		//static char NextKeyPress();
		static char PopKeyPress();
		static void PushKey( char param1 );
		RKPClass();
		static void Init();
		static void SendToPanel(byte* r, int nLen);  //send a message to the panel
		static void SendToPanelEx(byte* r, int len);
		static void SendToPanel(bool bAck );
		static char dispBuffer[];

		static bool DecodeScreen( byte* msgbuf, int msgbufLen ); //Renders the screen from compressed bytes to Text
                
		static bool dateFlash;
		
		static bool mbIsPanelWarning;
		static bool mbIsPanelAlarm;
    static bool bScreenHasUpdated; //When true there is a display buffer ready to go //TODO: get rid of this
		static unsigned long timeToSwitchOffLed;
    static byte lastkey;

};

#endif
