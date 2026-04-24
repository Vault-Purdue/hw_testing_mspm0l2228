# HOW TO USE THIS PROGRAM:
# When run, you will be prompted to enter a payload string. This string will be encoded and sent
# to the MSPM0 board over the specified UART port. (For me, that's COM5, but you may need to change
# that to the port your board is plugged into.)
# The program will then listen for responses from the board, printing out any received messages.
# You can change the MESSAGE_ID variable at the top to simulate sending different message types. 
# (see hsm/assets/docs/uart_protocol.md for more details on message types and payload formats)
# The payload can be up to 128 bytes long.
# Running the program once will allow you to send one message and receive responses to that message 
# until you exit with Ctrl+C. This could be easily changed if you want to send multiple messages in 
# one session, but it was helpful when I was testing to see every response to a single message at a 
# time.
# Currently, checksum validation is commented out because I haven't implemented the checksum generation
# on the HSM side yet.

#pip install pyserial

import serial
import time


PORT = 'COM5' # Change this to your port
MESSAGE_ID = 0x01 # Change this to simulate sending different message types.

BAUDRATE = 115200

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
    frame.append(len(payload))
    encoded_payload = payload.encode('utf-8')  # Convert string to bytes
    frame.extend(encoded_payload)
    frame.extend(crc16_ccitt(encoded_payload).to_bytes(2, 'big'))  # CRC16
    return frame

def main():
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    time.sleep(0.1) # Let the port settle
    ser.reset_input_buffer() # Clear any phantom bytes

    try:
        payload = input("Enter payload: ")
        if len(payload) > 128:
            print("Payload too long! Max 128 bytes.")
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
            
            header = ser.read(2)
            if len(header) < 2:
                print("Header timeout")
                continue

            print("Got header: ", header.hex())

            # Read message ID and length
            msg_id = header[0]
            length = header[1]
            #length = int.from_bytes(length_bytes, 'big
            #length = int(length)

            print("Got length: ", length)

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