#ifndef COAP_SERVER_H
#define COAP_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <coap-simple.h>  // Library: https://github.com/hirotakaster/CoAP-simple-library

void setupCoAP();
void coapLoop();
void ledResourceHandler(CoapPacket *packet, IPAddress ip, int port);

#endif
