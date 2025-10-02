import serial, re

def test_serial_format():
    ser = serial.Serial("COM3", 115200, timeout=2)  # Change COM port
    line = ser.readline().decode().strip()
    print("Sample:", line)

    # Expected format: time,ax,ay,az,gx,gy,gz,ir,drowsy
    pattern = r"^\d+,-?\d+\.\d+,-?\d+\.\d+,-?\d+\.\d+,-?\d+\.\d+,-?\d+\.\d+,-?\d+\.\d+,\d+,[01]$"
    assert re.match(pattern, line), "❌ Serial output invalid!"
    ser.close()
    print("✅ Serial format test passed.")
