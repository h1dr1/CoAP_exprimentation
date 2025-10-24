#!/usr/bin/env python3
"""
CoAP Client for ESP32 LED Control System with Dashboard
Requires: pip install aiocoap
"""

import asyncio
from aiocoap import *
import sys
from datetime import datetime

class CoapClient:
    def __init__(self, server_ip):
        self.server_ip = server_ip
        self.token = None
        self.username = None
    
    async def register(self, username, password):
        """Register a new user"""
        protocol = await Context.create_client_context()
        
        request = Message(
            code=POST,
            payload=f"{username}:{password}".encode('utf-8'),
            uri=f"coap://{self.server_ip}/register"
        )
        
        try:
            response = await protocol.request(request).response
            result = response.payload.decode('utf-8')
            print(f"✓ Register: {result}")
            return result.startswith("SUCCESS")
        except Exception as e:
            print(f"✗ Error: {e}")
            return False
    
    async def login(self, username, password):
        """Login and get authentication token"""
        protocol = await Context.create_client_context()
        
        request = Message(
            code=POST,
            payload=f"{username}:{password}".encode('utf-8'),
            uri=f"coap://{self.server_ip}/login"
        )
        
        try:
            response = await protocol.request(request).response
            result = response.payload.decode('utf-8')
            
            if result.startswith("TOKEN:"):
                self.token = result.split(":")[1]
                self.username = username
                print(f"✓ Login successful!")
                print(f"  Username: {username}")
                print(f"  Token: {self.token[:20]}...")
                return True
            else:
                print(f"✗ Login failed: {result}")
                return False
        except Exception as e:
            print(f"✗ Error: {e}")
            return False
    
    async def set_led(self, state):
        """Control LED (0=OFF, 1=ON)"""
        if not self.token:
            print("✗ Please login first!")
            return False
        
        protocol = await Context.create_client_context()
        
        request = Message(
            code=PUT,
            payload=f"{self.token}:{state}".encode('utf-8'),
            uri=f"coap://{self.server_ip}/led"
        )
        
        try:
            response = await protocol.request(request).response
            result = response.payload.decode('utf-8')
            
            if result.startswith("SUCCESS"):
                state_str = "ON" if state == 1 else "OFF"
                print(f"✓ LED turned {state_str}")
            else:
                print(f"✗ {result}")
            
            return result.startswith("SUCCESS")
        except Exception as e:
            print(f"✗ Error: {e}")
            return False
    
    async def get_led_state(self):
        """Get current LED state"""
        if not self.token:
            print("✗ Please login first!")
            return None
        
        protocol = await Context.create_client_context()
        
        request = Message(
            code=GET,
            payload=self.token.encode('utf-8'),
            uri=f"coap://{self.server_ip}/led"
        )
        
        try:
            response = await protocol.request(request).response
            result = response.payload.decode('utf-8')
            
            if result.startswith("STATE:"):
                state = result.split(":")[1]
                print(f"✓ LED is currently: {state}")
            else:
                print(f"✗ {result}")
            
            return result
        except Exception as e:
            print(f"✗ Error: {e}")
            return None
    
    async def get_dashboard(self):
        """Get dashboard data"""
        if not self.token:
            print("✗ Please login first!")
            return None
        
        protocol = await Context.create_client_context()
        
        request = Message(
            code=GET,
            payload=self.token.encode('utf-8'),
            uri=f"coap://{self.server_ip}/dashboard"
        )
        
        try:
            response = await protocol.request(request).response
            result = response.payload.decode('utf-8')
            
            # Parse dashboard data
            parts = result.split('|')
            print("\n" + "="*40)
            print("          DASHBOARD")
            print("="*40)
            
            for part in parts:
                key_val = part.split(':')
                if len(key_val) == 2:
                    key, val = key_val
                    print(f"  {key.ljust(12)}: {val}")
            
            print("="*40 + "\n")
            return result
        except Exception as e:
            print(f"✗ Error: {e}")
            return None
    
    async def get_logs(self):
        """Get activity logs"""
        if not self.token:
            print("✗ Please login first!")
            return None
        
        protocol = await Context.create_client_context()
        
        request = Message(
            code=GET,
            payload=self.token.encode('utf-8'),
            uri=f"coap://{self.server_ip}/logs"
        )
        
        try:
            response = await protocol.request(request).response
            result = response.payload.decode('utf-8')
            
            print("\n" + "="*60)
            print("                    ACTIVITY LOGS")
            print("="*60)
            
            if result.startswith("LOGS:"):
                logs_data = result.split(":")[1]
                
                if logs_data and logs_data != "No logs available":
                    logs = logs_data.split(';')
                    print(f"{'User':<15} {'Action':<10} {'Timestamp':<20}")
                    print("-"*60)
                    
                    for log in logs:
                        parts = log.split(',')
                        if len(parts) == 3:
                            user, state, timestamp = parts
                            # Convert milliseconds to readable format
                            time_str = f"{int(timestamp)/1000:.2f}s"
                            print(f"{user:<15} {state:<10} {time_str:<20}")
                else:
                    print("  No logs available yet")
            
            print("="*60 + "\n")
            return result
        except Exception as e:
            print(f"✗ Error: {e}")
            return None


async def interactive_menu(client):
    """Interactive menu for the CoAP client"""
    while True:
        print("\n" + "="*50)
        print("     CoAP LED Control Client - Main Menu")
        print("="*50)
        
        if client.token:
            print(f"  Logged in as: {client.username}")
        else:
            print(f"  Not logged in")
        
        print("\n  Authentication:")
        print("    1. Register new user")
        print("    2. Login")
        
        print("\n  LED Control:")
        print("    3. Turn LED ON")
        print("    4. Turn LED OFF")
        print("    5. Get LED State")
        
        print("\n  Information:")
        print("    6. View Dashboard")
        print("    7. View Activity Logs")
        
        print("\n  Other:")
        print("    8. Logout")
        print("    9. Exit")
        
        print("="*50)
        
        choice = input("\nEnter choice (1-9): ").strip()
        
        if choice == '1':
            print("\n--- Register New User ---")
            username = input("Username (min 3 chars): ")
            password = input("Password (min 3 chars): ")
            await client.register(username, password)
        
        elif choice == '2':
            print("\n--- Login ---")
            username = input("Username: ")
            password = input("Password: ")
            await client.login(username, password)
        
        elif choice == '3':
            await client.set_led(1)
        
        elif choice == '4':
            await client.set_led(0)
        
        elif choice == '5':
            await client.get_led_state()
        
        elif choice == '6':
            await client.get_dashboard()
        
        elif choice == '7':
            await client.get_logs()
        
        elif choice == '8':
            if client.token:
                client.token = None
                client.username = None
                print("✓ Logged out successfully")
            else:
                print("✗ Not logged in")
        
        elif choice == '9':
            print("\n👋 Goodbye!")
            break
        
        else:
            print("✗ Invalid choice! Please enter 1-9")
        
        input("\nPress Enter to continue...")


async def main():
    print("\n" + "="*50)
    print("  CoAP LED Control System - Client")
    print("="*50 + "\n")
    
    if len(sys.argv) < 2:
        print("Usage: python coap_client.py <ESP32_IP_ADDRESS>")
        print("Example: python coap_client.py 192.168.1.100")
        sys.exit(1)
    
    server_ip = sys.argv[1]
    print(f"Connecting to CoAP server at: {server_ip}:5683\n")
    
    client = CoapClient(server_ip)
    
    await interactive_menu(client)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\n👋 Interrupted by user. Goodbye!")
        sys.exit(0)

