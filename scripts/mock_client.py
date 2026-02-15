#!/usr/bin/env python3
import socket
import time
import math
import random

UDP_IP = "127.0.0.1"
UDP_PORT = 8080
HZ = 50 # Send 50 packets per second
SLEEP_TIME = 1.0 / HZ

def main():
    """
    Mock Client to generate noisy sinusoidal telemetry data.
    Sends data over UDP to the specified IP and Port.
    """
    # Create an IPv4 UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"Started Mock Client. Sending UDP packets to {UDP_IP}:{UDP_PORT} at {HZ} Hz...")
    print("Press Ctrl+C to stop.")
    
    t = 0.0
    try:
        while True:
            # 1. Generate a pure signal: 1 Hz sine wave
            pure_signal = math.sin(2 * math.pi * 1.0 * t)
            
            # 2. Add uniform random noise [-0.5, 0.5]
            noise = random.uniform(-0.5, 0.5)
            noisy_signal = pure_signal + noise
            
            # 3. Generate a timestamp in milliseconds
            timestamp_ms = int(time.time() * 1000)
            
            # 4. Format payload string: "timestamp:value"
            message = f"{timestamp_ms}:{noisy_signal:.6f}"
            
            # 5. Send packet
            sock.sendto(message.encode('utf-8'), (UDP_IP, UDP_PORT))
            
            # Step time forward and sleep to match the target frequency
            t += SLEEP_TIME
            time.sleep(SLEEP_TIME)
            
    except KeyboardInterrupt:
        print("\nMock Client stopped gracefully.")
    finally:
        sock.close()

if __name__ == "__main__":
    main()
