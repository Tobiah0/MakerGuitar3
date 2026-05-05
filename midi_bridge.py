import serial
import mido

SERIAL_PORT = '/dev/cu.usbserial-10' 
BAUD_RATE = 115200 #same as bord
MIDI_PORT_NAME = 'IAC Driver Bus 1' 

def main():
    print("Available MIDI Outputs:")
    for name in mido.get_output_names():
        print(f" - {name}")
    print("-" * 20)

    target_port = next((port for port in mido.get_output_names() if MIDI_PORT_NAME in port), None)
    
    if not target_port:
         print(f"Error: Could not find '{MIDI_PORT_NAME}'. Did you enable it in Audio MIDI Setup?")
         return

    try:
        # Open serial but prevent the ESP32 from getting stuck in a reset loop
        ser = serial.Serial()
        ser.port = SERIAL_PORT
        ser.baudrate = BAUD_RATE
        ser.setDTR(False) 
        ser.setRTS(False) 
        ser.open()
        
        print(f"Listening to Makerboard on {SERIAL_PORT}...")

        with mido.open_output(target_port) as midi_out:
            print(f"Forwarding MIDI to: {target_port}")
            print("Touch a fret to play! Press Ctrl+C to stop.\n")

            while True:
                if ser.in_waiting > 0:
                    try:
                        # raw lines
                        raw_line = ser.readline()
                        
                        # strip
                        line = raw_line.decode('utf-8').strip()
                        
                        # print EVERYTHING you get
                        print(f"[RAW SERIAL] {line}")
                        
                        if "," in line:
                            action, note_str = line.split(",")
                            note = int(note_str)

                            if action == "ON":
                                msg = mido.Message('note_on', note=note, velocity=127)
                                midi_out.send(msg)
                                print(f"🎵 Fret: Note {note} ON")
                                
                            elif action == "OFF":
                                msg = mido.Message('note_off', note=note, velocity=0)
                                midi_out.send(msg)
                                print(f"🔇 Fret: Note {note} OFF")
                                
                    except Exception as e:
                        print(f"[DEBUG ERROR] Could not parse data: {e}")

    except serial.SerialException:
        print(f"Error: Could not open {SERIAL_PORT}. Is the PlatformIO Serial Monitor closed?")
    except KeyboardInterrupt:
        print("\nClosing bridge.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    main()