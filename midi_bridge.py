import serial
import mido

# --- MAC CONFIGURATION ---
# Replace this with your exact ESP32 port (e.g., '/dev/cu.usbserial-0001')
SERIAL_PORT = '/dev/cu.wchusbserial110' 
BAUD_RATE = 115200

# The default name for the Mac IAC Driver port
MIDI_PORT_NAME = 'IAC Driver Bus 1' 

def main():
    print("Available MIDI Outputs:")
    for name in mido.get_output_names():
        print(f" - {name}")
    print("-" * 20)

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
        print(f"Listening to Makerboard on {SERIAL_PORT}...")

        # Find the IAC Driver port
        target_port = next((port for port in mido.get_output_names() if MIDI_PORT_NAME in port), None)
        
        if not target_port:
             print(f"Error: Could not find '{MIDI_PORT_NAME}'. Did you enable it in Audio MIDI Setup?")
             return

        with mido.open_output(target_port) as midi_out:
            print(f"Forwarding MIDI to: {target_port}")
            print("Ready to play! Press Ctrl+C to stop.\n")

            while True:
                # Read the incoming text from the ESP32
                if ser.in_waiting > 0:
                    try:
                        # Decode the serial bytes into a standard text string
                        line = ser.readline().decode('utf-8').strip()
                        
                        # Check if the string matches our "ACTION,NOTE" format
                        if "," in line:
                            action, note_str = line.split(",")
                            note = int(note_str)

                            if action == "ON":
                                msg = mido.Message('note_on', note=note, velocity=127)
                                midi_out.send(msg)
                                print(f"Strum: Note {note} ON")
                                
                            elif action == "OFF":
                                msg = mido.Message('note_off', note=note, velocity=0)
                                midi_out.send(msg)
                                print(f"Mute: Note {note} OFF")
                    except Exception as e:
                        # Catch any garbled serial data while plugging in
                        pass

    except serial.SerialException:
        print(f"Error: Could not open {SERIAL_PORT}. Is the PlatformIO Serial Monitor closed?")
    except KeyboardInterrupt:
        print("\nClosing bridge.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    main()