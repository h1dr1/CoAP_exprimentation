#ifndef COAP_SERVER_H
#define COAP_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <coap-simple.h>  // Library: https://github.com/hirotakaster/CoAP-simple-library
#include <map>
#include <vector>

// Function declarations
void setupCoAP();
void coapLoop();

// Resource handlers
void ledResourceHandler(CoapPacket *packet, IPAddress ip, int port);
void registerHandler(CoapPacket *packet, IPAddress ip, int port);
void loginHandler(CoapPacket *packet, IPAddress ip, int port);
void dashboardHandler(CoapPacket *packet, IPAddress ip, int port);
void logsHandler(CoapPacket *packet, IPAddress ip, int port);

// Utility functions
String generateToken(String username);
bool validateToken(String token);
String getUsernameFromToken(String token);
void addLEDLog(String username, String state);

// Data structures
struct User {
  String username;
  String password;
  String token;
  unsigned long tokenExpiry;
};

struct LEDLog {
  String username;
  String state;
  unsigned long timestamp;
};

// External declarations for global variables
extern std::map<String, User> users;
extern std::vector<LEDLog> ledLogs;
extern WiFiUDP udp;
extern Coap coap;

#endif
