#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

////////////////////  WIFI  ////////////////////
const char* WIFI_SSID = "hdr";
const char* WIFI_PASS = "12345678@";

////////////////////  LED  /////////////////////
const int LED_PIN = 2;

////////////////////  COAP  ////////////////////
WiFiUDP udp;
Coap coap(udp);

// Helper: set LED by string
bool apply_led_payload(const char* s) {
  String v = s;
  v.trim();
  v.toLowerCase();
  bool on = (v == "on" || v == "1" || v == "true");
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  
  Serial.print("🔌 LED set to: ");
  Serial.println(on ? "ON" : "OFF");
  
  return on;
}

// CoAP resource callback
void led_resource(CoapPacket& packet, IPAddress ip, int port) {
  char buf[packet.payloadlen + 1];
  memcpy(buf, packet.payload, packet.payloadlen);
  buf[packet.payloadlen] = '\0';

  Serial.println("╔═══════════════════════════════════════╗");
  Serial.print("║ 📥 CoAP Request from: ");
  Serial.print(ip);
  Serial.print(":");
  Serial.println(port);
  Serial.print("║    Message ID: ");
  Serial.println(packet.messageid);
  Serial.print("║    Token Length: ");
  Serial.println(packet.tokenlen);
  Serial.print("║    Type: ");
  Serial.println(packet.type);
  Serial.print("║    Code: ");
  Serial.println(packet.code);
  Serial.print("║    Method: ");
  
  switch (packet.code) {
    
    case COAP_GET:
      {
        Serial.println("GET");
        Serial.println("╠═══════════════════════════════════════╣");
        
        bool on = (digitalRead(LED_PIN) == HIGH);
        String state = (on ? "on" : "off");
        
        Serial.print("║ 📤 Response: ");
        Serial.println(state);
        Serial.println("║ 🔄 Sending response...");
        
        // Send response with explicit parameters
        int result = coap.sendResponse(ip, port, packet.messageid, state.c_str());
        
        Serial.print("║ ✅ sendResponse returned: ");
        Serial.println(result);
        Serial.println("╚═══════════════════════════════════════╝");
        break;
      }
    
    case COAP_PUT:
    case COAP_POST:
      {
        Serial.print(packet.code == COAP_PUT ? "PUT" : "POST");
        Serial.println("║");
        Serial.print("║    Payload: ");
        Serial.println(buf);
        Serial.println("╠═══════════════════════════════════════╣");
        
        bool on = apply_led_payload(buf);
        String reply = String("OK LED=") + (on ? "on" : "off");
        
        Serial.print("║ 📤 Response: ");
        Serial.println(reply);
        Serial.println("║ 🔄 Sending response...");
        
        int result = coap.sendResponse(ip, port, packet.messageid, reply.c_str());
        
        Serial.print("║ ✅ sendResponse returned: ");
        Serial.println(result);
        Serial.println("╚═══════════════════════════════════════╝");
        break;
      }
    
    default:
      Serial.println("UNSUPPORTED");
      Serial.println("╠═══════════════════════════════════════╣");
      Serial.println("║ 📤 Response: Method Not Allowed");
      
      coap.sendResponse(ip, port, packet.messageid, "Method Not Allowed");
      Serial.println("╚═══════════════════════════════════════╝");
      break;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n\n");
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║   🚀 ESP32 CoAP LED Control v2.1     ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.println();

  // Test LED
  Serial.println("🔦 Testing LED...");
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  Serial.println("✅ LED test complete\n");

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("📡 Connecting to WiFi...");
  Serial.print("   SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("   ");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.println("║         ✅ WiFi Connected!            ║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.println();
    Serial.print("📍 IP Address:      ");
    Serial.println(WiFi.localIP());
    Serial.print("🌐 Gateway:         ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("🔗 Subnet Mask:     ");
    Serial.println(WiFi.subnetMask());
    Serial.print("📡 MAC Address:     ");
    Serial.println(WiFi.macAddress());
    Serial.print("📶 Signal:          ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println();
    
  } else {
    Serial.println("❌ WiFi Connection Failed!");
    Serial.println("Restarting...");
    delay(3000);
    ESP.restart();
    return;
  }

  // Start CoAP
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║      🔧 Starting CoAP Server...      ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.println();
  
  // Initialize UDP with explicit port
  if (udp.begin(5683)) {
    Serial.println("✅ UDP socket opened on port 5683");
  } else {
    Serial.println("❌ Failed to open UDP socket!");
  }
  
  coap.server(led_resource, "led");
  coap.start();
  
  Serial.println("✅ CoAP server started");
  Serial.println();
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║         📋 CoAP Endpoint:            ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.print("   coap://");
  Serial.print(WiFi.localIP());
  Serial.println(":5683/led");
  Serial.println();
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║    ✅ Ready - Waiting for requests    ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.println();
  Serial.println("💡 TIP: Test with command:");
  Serial.print("   coap get coap://");
  Serial.print(WiFi.localIP());
  Serial.println(":5683/led");
  Serial.println();
}

void loop() {
  coap.loop();
  
  // Debug info every 30 seconds
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 30000) {
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("💓 System Status:");
    Serial.print("   WiFi: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("   LED: ");
    Serial.println(digitalRead(LED_PIN) ? "ON" : "OFF");
    Serial.print("   Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    lastDebug = millis();
    
    // Reconnect WiFi if needed
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️  WiFi disconnected! Reconnecting...");
      WiFi.reconnect();
    }
  }
}
