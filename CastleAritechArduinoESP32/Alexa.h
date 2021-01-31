// Alexa.h

#ifndef _ALEXA_h
#define _ALEXA_h

#include <HTTPServer.hpp>

void AlexaStart(httpsserver::HTTPServer* secureServer);

void AlexaLoop();



#endif

