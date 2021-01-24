/*
  SMTP.cpp

  Created: 4/5/2014 7:45:36 PM
  Author: Ambrose

  SENDEMAILS defined in config.h determines if emails will be sent
*/

#include <Arduino.h>

#include "limits.h"
#include "SMTP.h"
#include "Log.h"
#include "Config.h"
#include "WebSocket.h" //for the base64 stuff

#ifdef VM
  #include "libs\WiFi\WiFi.h"
  #include "libs\WiFiClientSecure\WiFiClientSecure.h"
#else
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#endif

int SMTP::nEmailStage = 0;

#ifndef SENDEMAILS

// protected constructor
SMTP::SMTP(){}
boolean SMTP::WaitForReplyLine(){}
void SMTP::QueueEmail(MSG a){}
void SMTP::Init(){}
void SMTP::SendEmailProcess(){}

#else
MSG SMTP::nMsgToSend = MSG_NA;

SMTP::SMTP(){}
void SMTP::QueueEmail(MSG msgToSend)
{
  LogLn(F("Queue Email."));
  SMTP::nMsgToSend = msgToSend;
  SMTP::nEmailStage = 1;
}

String _error="";
String _serverResponse="";



void a3_to_a4(unsigned char* a4, unsigned char* a3) {
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);
}

int base64_encode(char* output, char* input, int inputLen)
{
    int i = 0, j = 0;
    int encLen = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    static char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    while (inputLen--) {
        a3[i++] = *(input++);
        if (i == 3) {
            a3_to_a4(a4, a3);

            for (i = 0; i < 4; i++)
                output[encLen++] = b64_alphabet[a4[i]];
            i = 0;
        }
    }
    if (i) {
        for (j = i; j < 3; j++)
            a3[j] = '\0';

        a3_to_a4(a4, a3);

        for (j = 0; j < i + 1; j++)
            output[encLen++] = b64_alphabet[a4[j]];

        while ((i++ < 3))
            output[encLen++] = '=';
    }
    output[encLen] = '\0';
    return encLen;
}

bool AwaitSMTPResponse(WiFiClientSecure& client, const String& resp = "", uint16_t timeOut = 10000)
{
    uint32_t ts = millis();
    while (!client.available())
    {
        if (millis() > (ts + timeOut)) {
            LogLn("SMTP Response TIMEOUT!");
            return false;
        }
        delay(100);
    }
    _serverResponse = client.readStringUntil('\n');

    LogLn("Resp->" + _serverResponse);

    if (resp.length()>0 && _serverResponse.indexOf(resp) == -1)
        return false;
    return true;
}

int nErrorCount = 0;
bool SendEmailWorkflow()
{
    //Serial.println(SMTP::nEmailStage);
    if (SMTP::nEmailStage == 0)
    {//Nothing to send...
        return true;
    }
    //-]Serial.println("Email to send");
    if (WiFi.status() != WL_CONNECTED)
    {
        //-]LogLn("No Wifi");
        return false; //Error: No Wifi
    }
    
    unsigned long tiStart = millis();

    WiFiClientSecure client;
    LogLn("Connecting to : " SMTP_SERVER);

    if (!client.connect(SMTP_SERVER, SMTP_PORT)) {
        LogLn("Could not connect to mail server");
        return false;
    }
    if (!AwaitSMTPResponse(client, "220")) {
        LogLn("Connection Error");
        return false;
    }

    LogLn("HELO friend:");
    client.println("HELO friend");
    if (!AwaitSMTPResponse(client, "250")) {
        LogLn("Identification error");
        return false;
    }

    LogLn("AUTH LOGIN:");
    client.println("AUTH LOGIN");
    AwaitSMTPResponse(client);

    char buffer[100]; //100 enough?
    {
        LogLn("EMAILBASE64_LOGIN:");
        const char* pUSER = SMTP_USER;
        base64_encode(buffer, (char*)pUSER, strlen(pUSER));
        client.println(buffer);
        AwaitSMTPResponse(client);
    }
    {
        LogLn("EMAILBASE64_PASSWORD:");
        const char* pPASS = SMTP_PASS;
        char buffer[100]; //100 plenty
        base64_encode(buffer, (char*)pPASS, strlen(pPASS)); //reuse htmlline buffer
        client.println(buffer);
        if (!AwaitSMTPResponse(client, "235")) {
            LogLn("SMTP AUTH error");
            return false;
        }
    }

    String mailFrom = "MAIL FROM: <" SMTP_USER ">";
    LogLn(mailFrom);
    client.println(mailFrom);
    AwaitSMTPResponse(client);

    String rcpt = "RCPT TO: <" EMAIL_ADDR ">";
    Serial.println(rcpt);
    client.println(rcpt);
    AwaitSMTPResponse(client);

    LogLn("DATA:");
    client.println("DATA");
    if (!AwaitSMTPResponse(client, "354")) {
       LogLn("SMTP DATA error");
       return false;
    }

    {//send the message body
        client.println("From: <" SMTP_USER ">");
        client.println("To: <" EMAIL_ADDR ">");
        client.print("Subject: ");
        client.println(EMAIL_SUBJECT);

        client.println("Content-Type: text/html; charset=\"UTF-8\"");
        client.println("Content-Transfer-Encoding: 7bit");
        client.println();
        //String body = "<!DOCTYPE html><html lang=\"en\">" + message + "</html>";

        String sMsg = "";
        if (SMTP::nMsgToSend == MSG_START)
            sMsg = "The House Alarm has just powered on.";
        else if (SMTP::nMsgToSend == MSG_ALARM)
            sMsg = "The House Alarm has gone off.";
        sMsg = "<div style=\"color:#ff0000;font-size:20px;\">" + sMsg + "</div>";
        client.println(sMsg);
        client.println(".");
        if (!AwaitSMTPResponse(client, "250")) {
            LogLn("Sending message error");
            return false;
        }
    }
    client.println("QUIT");
    if (!AwaitSMTPResponse(client, "221")) {
        LogLn("SMTP QUIT error");
        return false;
    }

    SMTP::nEmailStage = 0; //success - mark email as sent
    Serial.print("Time to send Email (ms): ");
    Serial.println(millis() - tiStart);
    return true;
}

void SendEmailThread(void* parameter)
{
    while(true)
    {
        if (SendEmailWorkflow()){
           delay(1000); //all ok - wait 1 sec before checking again
           nErrorCount=0;
        }
        else
        {
            nErrorCount ++;
            delay(10000); //there was an error - wait 10 seconds and retry
            if (nErrorCount > 3)
            {
                LogLn("Restart Wifi");
                if (WebSocket::nConnectState != WIFI_PENDING)
                    WebSocket::nConnectState = WIFI_DOWN;
            }
        }
    }
}

void SMTP::StartEmailMonitor()
{
    xTaskCreatePinnedToCore(
        SendEmailThread,
        "thread2",  // Task name
        50000,             // Stack size (bytes)
        NULL,             // Parameter
        1,                // Task priority
        NULL,             // Task handle
        1                 //ARDUINO_RUNNING_CORE
    );
}


#endif

