#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

////////////////////  WIFI  ////////////////////
const char* WIFI_SSID = "abc";
const char* WIFI_PASS = "12341234";

////////////////////  LED  /////////////////////
const int LED_PIN = 2;  // change if your board uses another LED pin

////////////////////  COAP  ////////////////////
WiFiUDP udp;
Coap coap(udp);  // default UDP 5683

// helper: set LED by string ("on","off","1","0","true","false")
bool apply_led_payload(const char* s) {
  String v = s;
  v.trim();
  v.toLowerCase();
  bool on = (v == "on" || v == "1" || v == "true");
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  return on;
}

// NOTE: signature uses REFERENCE, not pointer
void led_resource(CoapPacket& packet, IPAddress ip, int port) {
  // payload is not null-terminated, so make a safe buffer
  char buf[packet.payloadlen + 1];
  memcpy(buf, packet.payload, packet.payloadlen);
  buf[packet.payloadlen] = '\0';

  switch (packet.code) {
    case COAP_GET:
      {
        bool on = (digitalRead(LED_PIN) == HIGH);
        String reply = String("LED=") + (on ? "on" : "off");
        coap.sendResponse(ip, port, packet.messageid, reply.c_str());
        break;
      }
    case COAP_PUT:
    case COAP_POST:
      {
        bool on = apply_led_payload(buf);
        String reply = String("OK LED=") + (on ? "on" : "off");
        coap.sendResponse(ip, port, packet.messageid, reply.c_str());
        break;
      }
    default:
      coap.sendResponse(ip, port, packet.messageid, "Method Not Allowed");
      break;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Register /led (note: callback is by reference) and start server
  coap.server(led_resource, String("led"));
  coap.start();
}

void loop() {
  coap.loop();
}
