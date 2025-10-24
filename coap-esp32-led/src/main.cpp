#include "coap_server.h"

void setup() {
  // Initialize the CoAP server and WiFi connection
  setupCoAP();
}

void loop() {
  // Keep the CoAP server running
  coapLoop();
}
