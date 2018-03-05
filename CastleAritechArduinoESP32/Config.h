/*
 * Config.h
 *
 *
 *	This is all you should need edit to get a working system
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

//--------Configuration Start----------

#ifndef CONFIG_H_  //Dont touch
#define CONFIG_H_ 1  //Dont touch

//Set to 1 for normal operation - comment out will debug print useful logs - Only Useful for Leonardo
//#define QUIET 1  //NOTE - For Arduino UNO - THIS MUST BE DEFINED (as we need the only Serial port for panel comms)

#define RKP_ID 1  //This is the Keypad id of the Arduinio - recommended 1 or 2 (0 is first keypad)

//#define DUMP_RAW_LINE_DATA //This will dump all data from Arduino in hex to screen but no tx from arduino

//#Hardware pins
#define SERIAL1_RXPIN 13
#define SERIAL1_TXPIN 15
#define LED_Stat 14  //displays packets sent using LED 14 (Red Led)
#define ledFeedback 12 //Blink a Led on Pin 12 we can use to show ESP is running (Green Led)

#define ECHO_TEST_RTS -1 // (18) //unused but required to be defined can be: UART_PIN_NO_CHANGE
#define ECHO_TEST_CTS -1 // (19) //unused but required to be defined can be: UART_PIN_NO_CHANGE


//The Arduino IP address and Port (192.168.1.205:8383)
#define IP_A 192
#define IP_B 168
#define IP_C 0
#define IP_D 177 //205
#define IP_P 80  //8383       //The IP Port for the server to listen on

//#define SENDEMAILS 1      //Define this to enable sending emails - Comment line out to disable sending emails //TODO: reimplement

//Wifi Password //TODO: Move to config
#define WIFI_SSID "{WIFI NAME HERE}"
#define WIFI_PASSWORD "{WIFI PASS HERE}"

#define EMAIL_ADDR "example@example.com"  //Email to send to

#define USE_SMTP_PASSWORD                 //comment this out if your SMTP server doesnt need to logon before sending emails (eg. your ISP smtp server wont need login)
#define SMTP_USER "******@gmx.com"      //SMTP account name to send from (www.gmx.com is a good option)
#define SMTP_PASS "******"            //SMTP account password to send from



//Build Version (displayed on webpage)
#define sVersion "V5.01"


//The Email server
//This is the IP for "smtp.gmx.com" a free smtp server you can use
// smtp.gmx.com = 212.227.17.174
#define SMTP_IP_A 212
#define SMTP_IP_B 227
#define SMTP_IP_C 17
#define SMTP_IP_D 174


/*---To Send emails You can use gmx or you can use one of these IP address of the SMTP server that matches your Internet Provider
smtp.mysmart.ie							Smart Telecom Outgoing SMTP Server
smtp.irishbroadband.ie.					Irish Broadband Outgoing SMTP Server
mail1.eircom.net OR mail2.eircom.net	Eircom Outgoing SMTP Server
smtp.magnet.ie							Magnet Outgoing SMTP Server
smtp.upcmail.ie							NTL and UPC Outgoing SMTP Server
mail.icecomms.net						Ice Broadband Outgoing SMTP Server
mail.vodafone.ie						Vodafone Outgoing SMTP Server
smtp.o2.ie								O2 Outgoing SMTP Server
smtp.clearwire.ie / mail.clearwire.ie	Clearwire Outgoing SMTP Server
smtp.digiweb.ie							Digiweb Outgoing SMTP Server
mail.imagine.ie OR mail.gaelic.ie		Imagine Broadband Outgoing SMTP Server
mail.perlico.ie							Perlico Outgoing SMTP Server
mail-relay.3ireland.ie					3 Outgoing SMTP Server: Mobile broadband with 3 mobile Ireland
*/


#endif /* CONFIG_H_ */
