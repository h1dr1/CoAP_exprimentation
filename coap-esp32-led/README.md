# CoAP Server on ESP32 with LED Control

This project demonstrates how to create a CoAP (Constrained Application Protocol) server on an ESP32 microcontroller to expose and control an LED resource. CoAP is a lightweight protocol designed for machine-to-machine (M2M) communication over low-power networks. It mirrors the RESTful principles of HTTP but with less overhead, making it ideal for constrained devices like the ESP32.

## Pedagogical Objective
The goal of this project is to build a CoAP server that controls a resource (in this case, an LED) through CoAP requests. By the end of this project, you'll understand how the RESTful model is adapted for constrained networks and how to work with CoAP on low-power devices.

## Key Concepts
- **CoAP**: CoAP (Constrained Application Protocol) is a lightweight M2M protocol designed for IoT devices. It uses HTTP-like methods (GET, PUT, POST, DELETE) but over UDP to minimize overhead and power consumption.
- **UDP**: CoAP runs over UDP, which reduces connection overhead and power consumption. CoAP compensates for UDP's unreliability with a lightweight reliability mechanism, using "Confirmable" messages.
- **Resource-Oriented Architecture**: A CoAP device exposes its capabilities as resources (e.g., `/led`, `/sensors/temperature`). Interaction is done by making requests to these resources.

## Hardware Required (For Now)
- **1x ESP32** microcontroller
- **1x LED**

## Project Overview
The ESP32 will act as a CoAP server and expose a resource (`/led`) to control the LED.

### Server-Side Logic:
1. **GET request**: A GET request to `coap://[ESP32_IP]/led` returns the current state of the LED ("ON" or "OFF").
2. **PUT request**: A PUT request with a payload of "1" turns the LED ON, and a payload of "0" turns it OFF.

### Client-Side:
- Use a CoAP client tool (e.g., **libcoap command-line tool** or a browser extension) to send requests to the ESP32 IP and verify the control of the LED.

## How to Use

### 1. Setup the CoAP Server
- Program the ESP32 using the provided code to act as a CoAP server.
- The server will expose the `/led` resource, which can be interacted with via CoAP requests.

### 2. Testing the Server
- Use a CoAP client tool to test the functionality:
  - **GET** request: Check the current state of the LED.
    ```bash
    coap-client coap://[ESP32_IP]/led
    ```
  - **PUT** request: Turn the LED ON or OFF by sending a payload of "1" or "0".
    ```bash
    coap-client -m PUT coap://[ESP32_IP]/led 1  # Turns ON the LED
    coap-client -m PUT coap://[ESP32_IP]/led 0  # Turns OFF the LED
    ```

### 3. Code Structure
The code is divided into two parts:
- **Server Code**: The ESP32 code to handle CoAP requests and control the LED.
- **Client Interaction**: A script or instructions for using a CoAP client to interact with the ESP32.

## Requirements
- **ESP32 Board**: You will need to install the ESP32 libraries and set up the Arduino IDE or ESP-IDF for programming.
- **CoAP Client**: You can use the **libcoap** command-line tool or a browser extension that supports CoAP.

## Getting Started
1. **Flash the ESP32**: Flash the ESP32 with the provided code that implements the CoAP server logic.
2. **Install CoAP Client**: On your PC, install the `libcoap` tool or a CoAP-enabled browser extension.
3. **Test the CoAP Server**: Send requests from the client to the ESP32 and control the LED.

## License
This project is licensed under the MIT License.
