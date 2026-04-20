#pip install pyserial

import serial
import time

# Change this to your port
PORT = 'COM5'          # e.g. '/dev/ttyUSB0'
BAUDRATE = 115200      # Must match your MSPM0 config

MESSAGE_ID = 0x00 # Change this to simulate sending different message types.

def crc16_ccitt(data: bytes, poly=0x1021, init=0xFFFF):
    crc = init

    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF  # keep it 16-bit

    return crc

def construct_message(payload):
    frame = bytearray()
    frame.append(0xAA) # Start byte
    frame.append(MESSAGE_ID) # Message ID
    if len(payload) > 255:
        frame.append(len(payload))
    else:
        frame.append(0)
        frame.append(len(payload))
    encoded_payload = payload.encode('utf-8')  # Convert string to bytes
    frame.extend(encoded_payload)
    frame.extend(crc16_ccitt(encoded_payload).to_bytes(2, 'big'))  # CRC16
    return frame

def main():
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    time.sleep(0.1) # Let the port settle
    ser.reset_input_buffer() # Clear any phantom bytes

    # Give the board time to reset (common for USB CDC devices)
    #time.sleep(2)

    try:
        payload = input("Enter payload: ")
        if len(payload) > 1024:
            print("Payload too long! Max 1024 bytes.")
            return

        frame = construct_message(payload)
        print(f"Sending frame: {frame.hex()}")

        # Send payload
        ser.write(frame)

        while True:
            # Wait for SOF
            while (True):
                byte = 0x00
                byte = ser.read(1)
                if len(byte) == 0:
                    continue
                if byte[0] == 0xAA:
                    print("Got SOF: ", byte.hex())
                    break
            
            header = ser.read(3)
            if len(header) < 3:
                print("Header timeout")
                continue

            print("Got header: ", header.hex())

            # Read message ID and length
            msg_id = header[0]
            length_bytes = header[1:3]
            length = int.from_bytes(length_bytes, 'big')

            # Read payload
            payload = ser.read(length)

            # Read Checksum
            crc_received = ser.read(2)
            #crc_calculated = crc16_ccitt(payload).to_bytes(2, 'big')
            #if crc_received != crc_calculated:
            #    print("CRC mismatch! Received:", crc_received.hex(), "Calculated:", crc_calculated.hex())
            #    continue

            print(f"Received message ID: {hex(msg_id)}, Length: {length}, Payload: {payload.hex()}, Checksum: {crc_received.hex()}")
            print(f"Response Payload (ASCII): {payload.decode('utf-8', errors='replace')}")

    except KeyboardInterrupt:
        print("\nExiting...")

    finally:
        ser.close()

if __name__ == "__main__":
    main()