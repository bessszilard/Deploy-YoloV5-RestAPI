import socket
import tqdm
from PIL import Image
import torch
import io

LOCAL_DEBUG = False

model = torch.hub.load('ultralytics/yolov5', 'yolov5s')

def process_package():
    received_str = client.recv(1024).decode()
    if len(received_str) == 0:
        return
    print(f"string:{received_str}")
    file_name, file_size = received_str.split(";")
    print(file_name)
    file = open(file_name, "wb")
    file_bytes = b""

    done = False

    progress = tqdm.tqdm(unit="B", unit_scale=True,
                        unit_divisor=1000, total=int(file_size))

    while not done:
        data = client.recv(1024)
        if b"<END>" in data:
            file_bytes += data[:-5]
            done = True
        else:
            file_bytes += data
        progress.update(1024)

    print('image received')

    img = Image.open(io.BytesIO(file_bytes))


    results = model(img, size=640) # reduce size=320 for faster inference

    # print(results.xyxy[0].cpu().numpy())

    # file.write(file_bytes)
    results = model([img])
    results.render()  # updates results.imgs with boxes and labels
    # Image.fromarray(results.ims[0]).save("res.jpg")

    results = results.xyxy[0].cpu().numpy()

    resultCsv = "top_left_x;topleft_y;bottom_right_x;bottom_right_y;conf;label\n"
    for result in results:
        assert(result.size == 6)
        resultLine = f"{int(result[0])};{int(result[1])};{int(result[2])};{int(result[3])};{result[4]:0.4f};{int(result[5])}\n"
        
        if LOCAL_DEBUG:
            print(resultLine)
        resultCsv += resultLine

    print(resultCsv)

    response = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        response.connect(('localhost', 5000))
        response.sendall(resultCsv.encode())  
    except:
        print("Failed to connect host 5000")
    file.close()
    response.close()

if LOCAL_DEBUG:
    img = Image.open("image.jpg")

    # file.write(file_bytes)
    results = model([img])
    results.render()  # updates results.imgs with boxes and labels
    # Image.fromarray(results.ims[0]).save("res.jpg")

    results = results.xyxy[0].cpu().numpy()

    resultCsv = "top_left_x;topleft_y;bottom_right_x;bottom_right_y;conf;label\n"
    for result in results:
        assert(result.size == 6)
        resultLine = f"{int(result[0])};{int(result[1])};{int(result[2])};{int(result[3])};{result[4]:0.4f};{int(result[5])}\n"
        
        if LOCAL_DEBUG:
            print(resultLine)
        resultCsv += resultLine

    print(resultCsv)

if LOCAL_DEBUG == False:
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        server.bind(('localhost', 5005))
    except socket.error:
        server.bind(('localhost', 5006))
    print(f"Server port: {server.getsockname()[1]}")
    server.listen(5)
    client, addr = server.accept()

    try:
        while True:
            process_package()
            
    except KeyboardInterrupt:
        client.close()
        server.close()

   
   

