import serial

# Testar a conexão
ser = serial.Serial('COM6', 115200, timeout=1)
print("Conectado à porta serial")
ser.close()