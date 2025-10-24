#include "coap_server.h"

// WiFi credentials (edit to match your network)
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// LED pin
#define LED_PIN 2  // Built-in LED for most ESP32 boards

// CoAP server instance
WiFiUDP udp;
Coap coap(udp);

// ===== IN-MEMORY DATABASE =====
std::map<String, User> users;      // username -> User
std::vector<LEDLog> ledLogs;       // LED state change logs
const int MAX_LOGS = 50;

// ===== UTILITY FUNCTIONS =====
String generateToken(String username) {
  return username + "_" + String(millis()) + "_" + String(random(1000, 9999));
}

bool validateToken(String token) {
  for (auto& pair : users) {
    if (pair.second.token == token && millis() < pair.second.tokenExpiry) {
      return true;
    }
  }
  return false;
}

String getUsernameFromToken(String token) {
  for (auto& pair : users) {
    if (pair.second.token == token) {
      return pair.second.username;
    }
  }
  return "";
}

void addLEDLog(String username, String state) {
  LEDLog log = {username, state, millis()};
  ledLogs.push_back(log);
  if (ledLogs.size() > MAX_LOGS) {
    ledLogs.erase(ledLogs.begin());
  }
}

// ===== CoAP RESOURCE HANDLERS =====

// POST /register - Register new user
// Payload: username:password
void registerHandler(CoapPacket *packet, IPAddress ip, int port) {
  if (packet->code == COAP_POST) {
    String payload = "";
    for (int i = 0; i < packet->payloadlen; i++) {
      payload += (char)packet->payload[i];
    }
    
    int colonPos = payload.indexOf(':');
    if (colonPos == -1) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid format");
      return;
    }
    
    String username = payload.substring(0, colonPos);
    String pwd = payload.substring(colonPos + 1);
    
    if (username.length() < 3 || pwd.length() < 3) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Min 3 chars");
      return;
    }
    
    if (users.find(username) != users.end()) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:User exists");
      return;
    }
    
    User newUser = {username, pwd, "", 0};
    users[username] = newUser;
    
    Serial.println("User registered: " + username);
    coap.sendResponse(ip, port, packet->messageid, "SUCCESS:Registered");
  }
}

// POST /login - Login user
// Payload: username:password
// Response: TOKEN:xyz
void loginHandler(CoapPacket *packet, IPAddress ip, int port) {
  if (packet->code == COAP_POST) {
    String payload = "";
    for (int i = 0; i < packet->payloadlen; i++) {
      payload += (char)packet->payload[i];
    }
    
    int colonPos = payload.indexOf(':');
    if (colonPos == -1) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid format");
      return;
    }
    
    String username = payload.substring(0, colonPos);
    String pwd = payload.substring(colonPos + 1);
    
    if (users.find(username) == users.end()) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:User not found");
      return;
    }
    
    if (users[username].password != pwd) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Wrong password");
      return;
    }
    
    // Generate token valid for 1 hour
    String token = generateToken(username);
    users[username].token = token;
    users[username].tokenExpiry = millis() + 3600000;  // 1 hour
    
    Serial.println("User logged in: " + username);
    coap.sendResponse(ip, port, packet->messageid, ("TOKEN:" + token).c_str());
  }
}

// PUT /led - Control LED (requires authentication)
// Payload: token:state (state = 0 or 1)
// GET /led - Get LED state (requires authentication)
// Payload: token
void ledResourceHandler(CoapPacket *packet, IPAddress ip, int port) {
  String payload = "";
  for (int i = 0; i < packet->payloadlen; i++) {
    payload += (char)packet->payload[i];
  }
  
  int colonPos = payload.indexOf(':');
  String token = (colonPos != -1) ? payload.substring(0, colonPos) : payload;
  
  if (!validateToken(token)) {
    coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid token");
    return;
  }
  
  String username = getUsernameFromToken(token);
  
  // Handle PUT requests to change LED state
  if (packet->code == COAP_PUT) {
    if (colonPos == -1) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid format");
      return;
    }
    
    String state = payload.substring(colonPos + 1);
    
    if (state == "1") {
      digitalWrite(LED_PIN, HIGH);
      addLEDLog(username, "ON");
      Serial.println(username + " turned LED ON");
      coap.sendResponse(ip, port, packet->messageid, "SUCCESS:LED ON");
    } else if (state == "0") {
      digitalWrite(LED_PIN, LOW);
      addLEDLog(username, "OFF");
      Serial.println(username + " turned LED OFF");
      coap.sendResponse(ip, port, packet->messageid, "SUCCESS:LED OFF");
    } else {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid state");
    }
  }
  
  // Handle GET requests to return LED state
  if (packet->code == COAP_GET) {
    String state = digitalRead(LED_PIN) ? "ON" : "OFF";
    coap.sendResponse(ip, port, packet->messageid, ("STATE:" + state).c_str());
  }
}

// GET /dashboard - Get dashboard data
// Payload: token
// Response: USER:username|LED:state|USERS:count|LOGS:count
void dashboardHandler(CoapPacket *packet, IPAddress ip, int port) {
  if (packet->code == COAP_GET) {
    String payload = "";
    for (int i = 0; i < packet->payloadlen; i++) {
      payload += (char)packet->payload[i];
    }
    
    if (!validateToken(payload)) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid token");
      return;
    }
    
    String username = getUsernameFromToken(payload);
    String ledState = digitalRead(LED_PIN) ? "ON" : "OFF";
    
    String response = "USER:" + username + 
                     "|LED:" + ledState + 
                     "|USERS:" + String(users.size()) + 
                     "|LOGS:" + String(ledLogs.size());
    
    Serial.println("Dashboard accessed by: " + username);
    coap.sendResponse(ip, port, packet->messageid, response.c_str());
  }
}

// GET /logs - Get LED activity logs
// Payload: token
// Response: LOGS:user1,state1,time1;user2,state2,time2;...
void logsHandler(CoapPacket *packet, IPAddress ip, int port) {
  if (packet->code == COAP_GET) {
    String payload = "";
    for (int i = 0; i < packet->payloadlen; i++) {
      payload += (char)packet->payload[i];
    }
    
    if (!validateToken(payload)) {
      coap.sendResponse(ip, port, packet->messageid, "ERROR:Invalid token");
      return;
    }
    
    String response = "LOGS:";
    int count = 0;
    
    // Return last 10 logs (most recent first)
    for (auto it = ledLogs.rbegin(); it != ledLogs.rend() && count < 10; ++it, ++count) {
      if (count > 0) response += ";";
      response += it->username + "," + it->state + "," + String(it->timestamp);
    }
    
    if (count == 0) {
      response = "LOGS:No logs available";
    }
    
    Serial.println("Logs accessed");
    coap.sendResponse(ip, port, packet->messageid, response.c_str());
  }
}

// ===== SETUP AND LOOP =====

void setupCoAP() {
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize Serial
  Serial.begin(115200);
  Serial.println();
  Serial.println("=================================");
  Serial.println("   CoAP LED Control Server");
  Serial.println("=================================");
  
  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Start CoAP server
  coap.server(5683);
  coap.start();
  
  // Register CoAP resources
  coap.serverCallback(registerHandler, "register");
  coap.serverCallback(loginHandler, "login");
  coap.serverCallback(ledResourceHandler, "led");
  coap.serverCallback(dashboardHandler, "dashboard");
  coap.serverCallback(logsHandler, "logs");
  
  Serial.println("\n=================================");
  Serial.println("CoAP Server Started on Port 5683");
  Serial.println("=================================");
  Serial.println("\nAvailable Endpoints:");
  Serial.println("  POST /register  - Register new user (username:password)");
  Serial.println("  POST /login     - Login user (username:password)");
  Serial.println("  PUT  /led       - Control LED (token:0|1)");
  Serial.println("  GET  /led       - Get LED state (token)");
  Serial.println("  GET  /dashboard - Get dashboard data (token)");
  Serial.println("  GET  /logs      - Get activity logs (token)");
  Serial.println("\n=================================");
  Serial.println("Server Ready. Waiting for requests...");
  Serial.println("=================================\n");
}

void coapLoop() {
  coap.loop();
}
