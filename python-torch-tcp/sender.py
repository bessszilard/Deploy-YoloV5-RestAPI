import os
import socket

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('localhost', 5005))

file = open("image.jpg", 'rb')
file_size = os.path.getsize("image.jpg")


client.sendall(f"received_image.png;{str(file_size)}".encode())

print(file_size)

data = file.read()
client.sendall(data)
client.send(b"<END>")

file.close()
client.close()