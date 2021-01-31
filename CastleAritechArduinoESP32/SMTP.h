/* 
* SMTP.h
*
* Created: 4/5/2014 7:45:36 PM
*/


#ifndef __SMTP_H__
#define __SMTP_H__

#include "Config.h"
#include "Arduino.h" //for boolean byte etc.

enum MSG{ MSG_NA, MSG_START, MSG_ALARM, MSG_WARNING};

class SMTP
{
//variables
public:
	static int nEmailStage;
	static MSG nMsgToSend;

protected:
private:

#ifdef SENDEMAILS
	static unsigned long mTimeout;
	static boolean bWaitForResponse;
	static IPAddress mSMTPServerIP;
	static unsigned long mStartDelay;
#endif

//functions
public:
	static void QueueEmail(MSG msgToSend);
	static void StartEmailMonitor();
        
protected:
private:
	SMTP();
}; //SMTP

#endif //__SMTP_H__
