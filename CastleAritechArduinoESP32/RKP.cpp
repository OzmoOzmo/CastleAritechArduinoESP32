/*
 * RKP.cpp - All the Remote Keypad comms and display commands
 *
 * Created: 3/30/2014 10:04:10 PM
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
 *
 * ref. Gen. Interrupts http://gammon.com.au/interrupts
 *      UART Interrupts http://www.electroons.com/blog/2013/02/interrupt-driven-uart-serial-communication-for-atmel-avr/
        Low Level UART  http://www.avrfreaks.net/forum/tut-soft-using-usart-serial-communications?name=PNphpBB2&file=viewtopic&t=45341
        Bits explained  http://maxembedded.com/2013/09/the-usart-of-the-avr/
*/

#include <Arduino.h>
#include "RKP.h"
#include "LOG.h"
#include "Websocket.h"
#include "SMTP.h"
#include "Config.h"


//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_system.h"
//#include "nvs_flash.h"
#include "driver/uart.h"
//#include "freertos/queue.h"
//#include "esp_log.h"
#include "soc/uart_struct.h"




#define nSerialBaudKP_RX 1953 // 1953 is a factor of the crystal used on the cpu
#define ixMaxPanel 40	//40 bytes enough

const char PROGMEM allmonths[] = {"JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC"};
const char PROGMEM alldays[] = {"SUNMONTUEWEDTHUFRISAT"};

bool RKPClass::dateFlash=true;  //TODO:
bool RKPClass::mbIsPanelWarning=false;
bool RKPClass::mbIsPanelAlarm=false;
unsigned long RKPClass::timeToSwitchOffLed = 0;

FIFO RKPClass::fifo;
byte RKPClass::lastkey = 0xFF;

#define DISP_BUF_LEN 16+1+2		//16 characters - space - AW (will be followed by a Terminator 0)
byte RKPClass::dispBuffer[DISP_BUF_LEN + 1]="Not Connected";

//There are 3 serial ports on ESP32 - we will use 2 (1 for debug - 1 for panel)
HardwareSerial Serial1(1);

#define BUF_SIZE (4)
uint8_t* sendData = (uint8_t*) malloc(BUF_SIZE);

#define DISP_BUF_LEN 16+1+2    //16 characters - space - AW (will be followed by a Terminator 0)

#define uart_num UART_NUM_1    //will uses the second uart

RKPClass::RKPClass()
{//never runs
}

void RKPClass::SerialInit()
{
  //Initialise the Serial Port - cannot use Serial1 as it tends to not send data immediately by clumps data together
  Serial1.begin(nSerialBaudKP_RX, SERIAL_8N1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  uart_intr_config_t uart_intr;
  uart_intr.intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M
                          | UART_RXFIFO_TOUT_INT_ENA_M
                          | UART_FRM_ERR_INT_ENA_M
                          | UART_RXFIFO_OVF_INT_ENA_M
                          | UART_BRK_DET_INT_ENA_M
                          | UART_PARITY_ERR_INT_ENA_M;
  uart_intr.rxfifo_full_thresh = 1; //UART_FULL_THRESH_DEFAULT,  //120 default!! agh! need receive 120 chars before we see them
  uart_intr.rx_timeout_thresh = 10; //UART_TOUT_THRESH_DEFAULT,  //too high?
  uart_intr.txfifo_empty_intr_thresh = 10; //UART_EMPTY_THRESH_DEFAULT //

  uart_intr_config(uart_num, &uart_intr);
  //Log2("Driver fix", (int)n2);
    
  /*
  uart_config_t uart_config = {
    .baud_rate = 1953, //nSerialBaudKP_RX,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1
  };
  
  //Configure UART1 parameters
  uart_param_config(uart_num, &uart_config);
  //Set UART1 pins(TX: IO4, RX: I05, RTS: IO18, CTS: IO19)
  uart_set_pin(uart_num, SERIAL1_TXPIN, SERIAL1_RXPIN, ECHO_TEST_RTS, ECHO_TEST_CTS);
  //Install UART driver with defintiely No event queue and no data send buffer
  uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);  //TODO: Buf_Size * 2????
  */
}

/*int RKPClass::SerialRead(byte* rx)
{
  //uint8_t buf[1]; 
  uint32_t length = 1;
  TickType_t ticks_to_wait = 1;
  int r = uart_read_bytes(uart_num, / *(uint8_t*) (&rx)* /rx, length, ticks_to_wait);
  return r;
}*/

//void SerialWrite(const char* buffer, uint32_t len)
//{
//  uart_tx_chars(uart_num, const char* buffer, uint32_t len);
//}


void RKPClass::Init()
{
  SerialInit();

  pinMode(LED_Stat, OUTPUT);
  /* Shows data clumping together when using default libraries - buffer issues
  while(true)
  {
      Serial1.write(0xFF);
      //delay(5);
      Serial1.write(0xFF);
      //delay(5);
      Serial1.write(0xFF);
      delay(100);
  }*/
}


//Decode screen to text and return true if necessary send to web client
bool RKPClass::DecodeScreen( byte* msgbuf, int msgbufLen )
{
	byte bufix=0;

	int ixMsgbuf=2;//skip 2 header bytes
	msgbufLen-=2; //remove checksum and zero terminator

	while(ixMsgbuf<msgbufLen)
	{
		byte rx = msgbuf[ixMsgbuf++];
		if (rx>=0 && rx < 0x0f)
		{//not implemented
		}
		else if (rx == 0x13)
		{//not implemented
		}
		else if (rx == 0x1b)
		{//to do with foreign character set - not implemented
		}
		else if (rx>= 0x20 && rx <= 0x7F)
		{//Normal ASCII
			if (bufix==0)
				//Force Screen clear at start of each message
				//for(int m=0;m<DISP_BUF_LEN;m++)
				//	dispBuffer[m]=0; //' ';
       memset(dispBuffer, 0, DISP_BUF_LEN);

			if (bufix < DISP_BUF_LEN)
				dispBuffer[bufix++]=(char)rx;
		}
		else if (rx>= 0x80 && rx <= 0x8F)
		{//Date in encoded format
			int b0=rx;
			int b1=msgbuf[ixMsgbuf++];
			int b2=msgbuf[ixMsgbuf++];
			int b3=msgbuf[ixMsgbuf++];

			byte nMonth= (b0 & 0x0f)-1;
			byte day = (b1 & (128+64+32))>> 5;
			byte date = (b1 & (31));
			byte h1=(b2 & 0xf0)>>4; if(h1==0x0A) h1=0;
			byte h2=(b2 & 0x0f); if(h2==0x0A) h2=0;
			byte m1=(b3 & 0xf0)>>4; if(m1==0x0A) m1=0;
			byte m2=(b3 & 0x0f); if(m2==0x0A) m2=0;

			memcpy_P(dispBuffer+0,alldays+(day*3),3);
			dispBuffer[3]=' ';
			dispBuffer[4]=('0'+(int)(date/10));
			dispBuffer[5]=('0'+(date%10));
			dispBuffer[6]=' ';

			memcpy_P(dispBuffer+7,allmonths+(nMonth*3),3);
			dispBuffer[10]=' ';
			dispBuffer[11]='0'+h1;
			dispBuffer[12]='0'+h2;
			//if (dateFlash) //too much web traffic
			dispBuffer[13]= ':';
			//else
			//	buffer[13]= F(' ');
			//dateFlash=!dateFlash;
			//buffer[13]= ((millis()/500)&1) ==0? ':':' ';
			dispBuffer[14]='0'+m1;
			dispBuffer[15]='0'+m2;
			bufix=0;
		}
		else if (rx == 0x90)
		{//CLS
			bufix=0;
			for(int m=0;m<DISP_BUF_LEN;m++)
				dispBuffer[m]=' ';
		}
		else if (rx == 0x91)
		{//HOME
			bufix=0;
		}
		else if (rx >= 0xA0 && rx <= 0xAf)
		{//MOVE cursor to position x
			bufix = (rx & 0x0f); //-1 gives us 2 *'s  but without -1 we go off screen at Login ***
		}
		else if (rx >= 0xB0 && rx <= 0xBF)
		{//{BLINK_N}"	Bxh Blink x chars starting at current cursor position
		 //not implementing this as it will cause unnecessary traffic sending display each second TODO: make underline maybe though
			//int nChars = (rx & 0x0f)-1;
			//if (dateFlash)
			//	buffer[i]= ':';
			//else
			//	buffer[i]= ' ';
			//dateFlash=!dateFlash;
		}
		else if (rx >= 0xC0 && rx <= 0xCf)
		{// Set position to x and clear all chars to right
			int i = (rx & 0x0f);
			if (i < DISP_BUF_LEN)
				bufix = i;
			for(int n=bufix;n<DISP_BUF_LEN;n++)
				dispBuffer[bufix++]=' ';
		}
		else if (rx>= 0xE0 && rx <= 0xFF)
		{// Special Characters Arrows and foreign chars
			int i = (rx & 0x0f);

			char c=0;
			if (i==4)	c= '*';
			else if (i==5)	c= 'V';
			else if (i==7)	c= '>';

			if (c>0)
				if (bufix < DISP_BUF_LEN)
					dispBuffer[bufix++]=c;
		}
		else
		{//unknown command
			Log(F("{"));LogHex(rx);Log(F("}"));
		}

		//Note: there are quite a few codes in Engineer menu to deal with flashing cursors and characters - cannot do easily in html
	}

	dispBuffer[16]='|';	//this may overwrite a char sometimes...acceptable.
	dispBuffer[17]=(RKPClass::mbIsPanelAlarm)?'A':' ';
	dispBuffer[18]=(RKPClass::mbIsPanelWarning)?'W':' ';

  //Checksum so can see if changes and need update client
  static int previousCS =-1;
	int cs=0;
	for(int n=0;n<DISP_BUF_LEN;n++)
		cs+=(char)dispBuffer[n];

	if (cs != previousCS)
	{
		////Display as hex
		//LogHex(msgbuf, msgbufLen);
		////Display as characters
		//for(int n=0;n<DISP_BUF_LEN;n++)	Log((char)dispBuffer[n]); LogLn(".");
		previousCS=cs;

		//Signal we have a new screen we can send
		return true; //ScreenHasUpdated
	}

	//LogHex(msgbuf,msgbufLen);  LogHex((byte*)dispBuffer,DISP_BUF_LEN);LogLn(".");
 return false;
}


char RKPClass::PopKeyPress()
{
  return (char)fifo.pop();
}

void RKPClass::PushKey( char key )
{
  fifo.push(key);
}

void RKPClass::SendDisplayToWebClient()
{
  //LogHex((byte*)dispBuffer,18);
  WebSocket::WebSocket_send((char*)dispBuffer, DISP_BUF_LEN);

  String strDisplay = String((char *)dispBuffer);
  ToScreen(0, strDisplay);
  Log("Sent:"); LogLn((char*)dispBuffer);
}


void RKPClass::Poll()
{
  #ifdef DUMP_RAW_LINE_DATA
    //each byte is about 5mS - so wait for 10mS before end of packet
    static byte buf[64];
    static int bufix=0;
    int tiLast=millis();
    while( millis()-tiLast < 10 )
    {
      while(Serial1.available())
      {
        tiLast=millis();
        buf[bufix++] = Serial1.read();
        if (bufix==64){LogHex(buf,bufix);bufix=0;}
      }
    }
    if (bufix>0){LogHex(buf,bufix);bufix=0;}
    return false;
  #else
    
    RKPClass::loop_PanelMon();
    if (RKPClass::loop_PanelMon())
      SendDisplayToWebClient();
  #endif
}

//Check see if UART has any data for us from panel - if a complete message was received parse it.
//Note - nothing here is to be blocking
//- so no delays or loops waiting for connections or data
bool RKPClass::loop_PanelMon()
{
  static int msgbufLen = 0;
  static byte msgbuf[ixMaxPanel];
  static bool bReceivingPacketNow = false;
  static byte lastByte=-1;
  bool bScreenHasUpdated = false;

  //Knock off the "We Sent To Panel" Led not less than 100ms after we turn it on
  if (timeToSwitchOffLed != 0 && millis() > timeToSwitchOffLed)
  {
    timeToSwitchOffLed = 0;
    digitalWrite(LED_Stat, LOW);
  }

  byte rx;
  
  //uint32_t length = 1;
  //TickType_t ticks_to_wait = 1;
  //int r = uart_read_bytes(uart_num, /*(uint8_t*) (&rx)*/rx, length, ticks_to_wait);
  //return r;
  
  //while(SerialRead(&rx) > 0) //rx passed by reference
  //while(uart_read_bytes(uart_num, &rx, 1, 1000) > 0)
  //size_t current_buf_len;
  //while (uart_get_buffered_data_len(uart_num, &current_buf_len) && current_buf_len > 0)

  while(Serial1.available())
  {
    byte rx = Serial1.read();
    //uart_read_bytes(uart_num, &rx, 1, 0);
    
    if (bReceivingPacketNow == false)
    {
      if(lastByte == 0)
      {//last char was a zero - this was the end of the last message
        if (rx==0)
          continue; //CD34 sends 0 at start And 0 at End - different to CD72 and 92 - so ignore extra zero

        //It may be for us or not - but get whole message before testing so we stay in sync
        bReceivingPacketNow = true;
        msgbufLen=0;
        //not necessary to blank - but good for debug
        for(int n=0;n<ixMaxPanel;n++)
          msgbuf[n]=0xFF;
        msgbuf[msgbufLen++]=rx;
      }
      lastByte = rx;
      continue; //wait for byte#3 (0,id,..)
    }
    lastByte = rx;  //needed in case cs error

    msgbuf[msgbufLen++]=rx;
    if (rx!=0)
    {//wasn't end of packet - is buffer full?
      if (msgbufLen>=ixMaxPanel)
      {//packet never terminated - bytes lost :(
        Log(F("Buf overflow"));
        bReceivingPacketNow = false;
        continue;
      }
    }
    else
    {//0 received - ie. End of the packet
      bReceivingPacketNow = false;

      byte idDev = (msgbuf[0] & 0xf0)>>4; //ID of remote that message is for 0x0-0xf are valid ids

      //Uncomment to Display all packets
      //#ifdef DISPLAY_ALL_PACKETS
      //Log(F("DevID:"));Log(idDev);LogLn(' ');LogHex(msgbuf,msgbufLen);
      //#endif

      byte cs = 0;
      for(int n=0;n<msgbufLen;n++)
      {
        byte rx = msgbuf[n];
        if (n<msgbufLen-2) //dont sum cs or terminator
          cs+=rx;
      }

      if (cs == 0)
        cs++; //protocol avoids sending a 0 (as its used for end marker(s)) - so will send cs 00 as 01
      if (cs != msgbuf[msgbufLen-2])
      {
        LogLn(F("CS Fail :( "));
        LogHex(msgbuf,msgbufLen);
        //Log(F("Dev:"));LogLn(idDev);LogLn(F("!"));
        continue;
      }

      //for us?
      if (idDev==RKP_ID)
      {
        bool bAck = (msgbuf[0] & 0x04) != 0;  //required for good comms
        if (!bAck)
          LogLn(F("Ack Fail")); //Too many of these will cause alarm to sound
                  
        SendToPanel(bAck);

        //ok - from here on until the next message we have a few ms to spare..
        if (RKPClass::DecodeScreen(msgbuf, msgbufLen))
        { //Queue up send to browser the screen as text
          bScreenHasUpdated = true;
        }
        //LogLn(F("-OK-"));
      }
      else
      {
        //LogLn("-Not for us-");
      }

      {//Send Email if alarm lights come on
        bool bIsPanelWarning = (msgbuf[1] & 0x04) != 0;
        if (bIsPanelWarning == true && RKPClass::mbIsPanelWarning == false)
          SMTP::QueueEmail(WARNING);
        RKPClass::mbIsPanelWarning = bIsPanelWarning;

        bool bIsPanelAlarm = (msgbuf[1] & 0x02) != 0;
        if (bIsPanelAlarm == true && RKPClass::mbIsPanelAlarm == false)
          SMTP::QueueEmail(ALARM);
        RKPClass::mbIsPanelAlarm = bIsPanelAlarm;
      }
    }
  }
  return bScreenHasUpdated;
}


//a 6 character keyboard buffer
byte FIFO::raw[maxkeybufsize];
FIFO::FIFO()
{
  nextIn = nextOut = count = 0;
}
void FIFO::push( byte element )
{
  if ( count >= maxkeybufsize )
  {
    Log("Too Full. Count=");LogLn(count);
    return; //lost
  }
  count++;
  raw[nextIn++] = element;
  nextIn %= maxkeybufsize;
  Log("Added Item. Count=");LogLn(count);
}

byte FIFO::pop()
{
  if (count>0)
  {
    count--;
    byte c=raw[ nextOut++];
    nextOut %= maxkeybufsize;
    
    //Log("Popped Item. Count=");Log(count);Log(" c=");LogLn(c);
    return c;
  }
  return 0xFF;
}


void RKPClass::SendToPanel( bool bAck )
{
	//Log(F("Send: "));
	#define H1_LIDTAMP 1
	#define H1_BASE 2 //always set - probably to make sure its not zero(start message)

  //We can emulate different keyboards - some keyboards have a few extra zones and outputs that we could use with Arduino sensors 
	#define H1_CD3008 0 //keypad with buzzer  - works on cd34 and cd72
	#define H1_CD9038 4 //Keypad with 4 zones - works on cd34 (TOTEST: if cd72 supported)
	#define H1_CD9041 8 //Glassbreak Expander with no keypad - works on cd34 not supported on cd72(Keypad displays Error)
	static byte nH2Previous=-1;

  static byte h[4];
  /*byte h1= &packetOut;
  byte h2= &packetOut+1;
  byte h3= &packetOut+2;
  byte h4= &packetOut+3;*/

	h[0] = H1_BASE + H1_CD9038 + (RKP_ID<<4); //kKKK (Capital K is an Extender enabled keypad)
	h[1] = 0;

	if (bAck)	//change to if(true) to remove retries
	{//LogLn("Ack");
		char nBrowserKeyPress = PopKeyPress();
		if (nBrowserKeyPress!=-1)
		{
      //You can use the pc keyboard as well as the on screen html buttons
			//nKeyToSend &= 0x20; //tolower
			int nNumb=0;
			if (nBrowserKeyPress >='1' && nBrowserKeyPress <= '9')
			nNumb = (nBrowserKeyPress-'0');
			else if (nBrowserKeyPress == '0')	nNumb = 0x0a;
			else if (nBrowserKeyPress == 'f'||nBrowserKeyPress == '*')	nNumb = 0x0b;	//UP  (* for IPhone)
			else if (nBrowserKeyPress == 'v'||nBrowserKeyPress == '#')	nNumb = 0x0c;	//DOWN (# for IPhone)
			else if (nBrowserKeyPress == 'p')	nNumb = 0x0d;	//UP + DOWN (Panic)
			else if (nBrowserKeyPress == 'x'||nBrowserKeyPress == ';'||nBrowserKeyPress == 'n'||nBrowserKeyPress == 'N')	nNumb = 0x0e;	//UP + 0 or X(reject) (WAIT on IPhone numpad)
			else if (nBrowserKeyPress == 13||nBrowserKeyPress == '+'||nBrowserKeyPress == 'y'||nBrowserKeyPress == 'Y')	nNumb = 0x0f;	//UP + 0 or *(accept) (+ on IPhone numpad)

			//Log(F("Sent: ")); LogHex(nNumb); LogHex(nBrowserKeyPress); LogLn(F("."));

			if(nNumb!=0)
				h[1] = nNumb<<4;
		}
		nH2Previous = h[1];
	}
	else
	{//If panel didnt get last kepress, resend it - if 0xFF or 0 then its a resend but the default normal we would have sent anyway.
		h[1] = (nH2Previous == 0xFF)? 0 : nH2Previous;
		{
			Log(F("Resend: ("));
			LogHex(h[1]);
			LogLn(F(")"));
		}
	}

	h[2]=0;
	h[3]=(byte)(h[0]+h[1]+h[2]);
  
  //uart_write_bytes(uart_num, (const char*)h, 4); 
  Serial1.write(h,4); //Serial1.flush();
  //uart_tx_chars(uart_num, (const char*)h, 4); //an even more direct write to the uart
  
  /*Serial1.write((byte)(h1));
  Serial1.write((byte)(h2));
  Serial1.write((byte)(h3));
  Serial1.write((byte)(h4));*/
  digitalWrite(LED_Stat, HIGH);
  timeToSwitchOffLed = millis() + 50;

	//Log("Send>");LogHex(h1); LogHex(h2); LogHex(h3); LogHex(h4); LogLn(".");
}


