import requests
f = open('../image.jpg', 'rb')

files = {"image": ("@image.jpg", f)}
# response = requests.post("http://127.0.0.1:5000/v1/object-detection/yolov5", files=files)
response = requests.post("http://127.0.0.1:3000/v1/object-detection/yolov5", files=files)

print(response.text)
print(response.content)