/*
 * Config.h
 *
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

#define RKP_ID 1  //This is the Keypad id of the Arduinio - recommended 1 or 2 (0 is first keypad)

#define LED_Stat 12 // Pin 14 on some ESP32s Pin 12 on D1 R32 - this binks when packets are sent to panel
#define ledFeedback 2 //Blink a Led on Pin 2 we can use to show ESP is running (Green Led)

//Wifi Password (Required)
#define WIFI_SSID "{WIFI NAME HERE}"
#define WIFI_PASSWORD "{WIFI PASS HERE}"
//Email Password (optional) are below in the email section


//The Arduino IP address and Port (192.168.1.177)
#define IP_ME "192.168.0.177"
#define IP_GW "192.168.0.1"
#define IP_SN "255.255.255.0"
#define IP_DNS IP_GW //This will use your ISP dns - if that doesnt work - try "8.8.8.8" (googles dns)

#define IP_P 80  //The IP Port for the http server

//Important: To use Gmail for sending emails via port 465 (SSL), Googles less secure app option must be enabled for that sending account.
//see for details: https://myaccount.google.com/lesssecureapps?pli=1
#define SMTP_SERVER "smtp.gmail.com"        
#define SMTP_PORT 465                   //always 465 for Secure Gmail
#define EMAIL_SUBJECT "House Alarm"
#define SMTP_USER "YOUREMAIL@gmail.com" //create a 
#define SMTP_PASS "PASSWORD"
#define EMAIL_ADDR "example@example.com"  //Email to send to




/////////
//These are advanced configuration...

//Switches - Comment out to disable features.. 
#define SENDEMAILS //Comment line out to disable sending emails
//#define DEBUG_LOG //Comment out to disable all debug comments
#define ENABLE_DISPLAY //Comment out to disable using LCD
//#define HTTPS //uncomment to use HTTPS (much slower and more limited browser support)
//#define DUMP_RAW_LINE_DATA //This will dump all data from Arduino in hex to debug log (DEBUG_LOG required)
//#define DISPLAY_ALL_PACKETS //for debugging comms - noramlly comment this line out (DEBUG_LOG required)
//#define SHOW_KEYPAD_SCREEN //Shows all screens in debug log - normally comment out (DEBUG_LOG required)
//#define VM //define this to use Visual Micro as the compiler (a faster compiler - but not free)
//#define REPORT_STACK_SPACE //Shows stats on the running threads 


//#Serial Port pins - We leave Serial0 for programming and debug - this serial port connects to the Aritech Panel via the circuit
//#define D1_R32  //uncomment this define to use the D1_R32 board
#if D1_R32
//These are the Pins to use for Serial Connection
#define SERIAL1_RXPIN 17
#define SERIAL1_TXPIN 16
#else
//Tested with "Wemos Lolin32" - These pins are more suited to the board layout
#define SERIAL1_RXPIN 13
#define SERIAL1_TXPIN 15
#endif

#define ECHO_TEST_RTS -1 // (18) //unused but required to be defined can be: UART_PIN_NO_CHANGE
#define ECHO_TEST_CTS -1 // (19) //unused but required to be defined can be: UART_PIN_NO_CHANGE

//Build Version (displayed on webpage)
#define sVersion "Castle V6.00"

//Maximum web browsers that can connect at a time
#define MAX_CLIENTS 20

//Symbols as displayed on the HTML page
#define KEY_STAR "&#9650;"  //unicode arrow up "*"
#define KEY_POUND "&#9660;" //unicode arrow down "#"


enum { WIFI_DOWN = 0, WIFI_PENDING = 1, WIFI_OK = 2 };


#endif /* CONFIG_H_ */

