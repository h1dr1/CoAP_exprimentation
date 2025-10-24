#include "coap_server.h"

// WiFi credentials (edit to match your network)
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// LED pin
#define LED_PIN 2  // Built-in LED for most ESP32 boards

// CoAP server instance
WiFiUDP udp;
Coap coap(udp);

// Resource handler for /led
void ledResourceHandler(CoapPacket *packet, IPAddress ip, int port) {
  String payload;

  // Handle PUT requests to change LED state
  if (packet->code == COAP_PUT) {
    for (int i = 0; i < packet->payloadlen; i++) {
      payload += (char)packet->payload[i];
    }

    if (payload == "1") {
      digitalWrite(LED_PIN, HIGH);
    } else if (payload == "0") {
      digitalWrite(LED_PIN, LOW);
    }

    Serial.print("LED set to: ");
    Serial.println(payload);
  }

  // Handle GET requests to return LED state
  if (packet->code == COAP_GET) {
    String state = digitalRead(LED_PIN) ? "ON" : "OFF";
    coap.sendResponse(ip, port, packet->messageid, state.c_str());
  }
}

// Setup WiFi and CoAP server
void setupCoAP() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Start CoAP server
  coap.server(5683);
  coap.start();

  // Add the /led resource
  coap.serverCallback(ledResourceHandler, "led");
  Serial.println("CoAP server started on port 5683");
}

// CoAP loop
void coapLoop() {
  coap.loop();
}
