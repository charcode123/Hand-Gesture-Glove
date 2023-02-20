import socket
import json

UDP_IP = "0.0.0.0"
UDP_PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
print("Running...")
serverAddressPort = ("127.0.0.1", 5433)
with socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM) as UDPClientSocket:
    while True:
        data, addr = sock.recvfrom(2048)
        # UDPClientSocket.sendto(data, serverAddressPort)
        json_data = json.loads(data.decode('utf-8'))
        print(json_data)
        json_data = json.dumps(json_data)
        UDPClientSocket.sendto(json_data.encode('utf-8'), serverAddressPort)
