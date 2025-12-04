#!/usr/bin/env python3

import asyncio
import argparse
import sys
from aiocoap import *

async def send_coap_request(server_ip, payload):
    """
    Asynchronously sends a CoAP PUT request with the given payload
    to the /led resource on the specified server IP.
    """
    
    # Create the CoAP client context
    context = await Context.create_client_context()
    
    # Construct the full URI
    uri_string = f'coap://{server_ip}/led'
    
    # Create the CoAP PUT message
    request = Message(
        code=PUT,
        payload=payload,
        uri=uri_string
    )

    try:
        # Send the request and wait for a response
        print(f"Sending payload '{payload.decode()}' to {uri_string}...")
        response = await context.request(request).response
        
        # Print the server's response
        print(f"Response Code: {response.code}")
        print(f" Response Payload: {response.payload.decode()}")

    except Exception as e:
        # Handle errors (e.g., server not found, timeout)
        print(f"An error occurred: {e}", file=sys.stderr)
        print("   Please check the IP address and ensure the ESP32 server is running.", file=sys.stderr)
        
    finally:
        # Clean up the CoAP context
        await context.shutdown()

def main():
    """
    Sets up the argument parser and runs the main CoAP logic.
    """
    
    # 1. Create the main parser
    parser = argparse.ArgumentParser(
        prog="control.py",
        description="CLI to control an ESP32 CoAP LED server.",
        epilog="Example: python3 control.py 192.168.1.104 on"
    )
    
    # 2. Add the IP address as a required positional argument
    parser.add_argument(
        "ip",  # The name of the argument
        type=str,
        help="The IP address of the ESP32 server."
    )
    
    # 3. Add Sub-commands for 'on' and 'off'
    # This creates a "sub-parser" that makes 'on' and 'off'
    # feel like built-in commands (e.g., like 'git push').
    subparsers = parser.add_subparsers(
        dest="action",      # The 'args' object will have an 'action' attribute
        required=True,    # The user MUST provide either 'on' or 'off'
        help="The action to perform."
    )
    
    # Create the 'on' command
    subparsers.add_parser("on", help="Turn the LED on (sends '1').")
    
    # Create the 'off' command
    subparsers.add_parser("off", help="Turn the LED off (sends '0').")

    # 4. Parse the arguments from the command line
    try:
        args = parser.parse_args()
    except SystemExit:
        # This catches errors like missing arguments and prints the help message
        return

    # 5. Determine the payload based on the chosen action
    if args.action == "on":
        payload = b"1"
    elif args.action == "off":
        payload = b"0"
    else:
        # This case should be unreachable thanks to 'required=True'
        print(f"Unknown action: {args.action}", file=sys.stderr)
        sys.exit(1)
        
    # 6. Run the asynchronous CoAP function
    # We use 'args.ip' and our determined 'payload'
    asyncio.run(send_coap_request(args.ip, payload))


if __name__ == "__main__":
    main()
