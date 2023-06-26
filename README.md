# Qt-deploy-YoloV5

Deploy YoloV5

## Used repository

* [GitHub - ultralytics/yolov5: YOLOv5 🚀 in PyTorch &gt; ONNX &gt; CoreML &gt; TFLite](https://github.com/ultralytics/yolov5)

* [GitHub - freshtechyy/ONNX-Runtime-GPU-image-classifciation-example](https://github.com/freshtechyy/ONNX-Runtime-GPU-image-classifciation-example)

## Tutorials

* [File Transfer via Sockets in Python - YouTube](https://www.youtube.com/watch?v=qFVoMo6OMsQ)

Python yolov5 flask docker file

[yolov5-flask/Dockerfile at master · robmarkcole/yolov5-flask · GitHub](https://github.com/robmarkcole/yolov5-flask/blob/master/Dockerfile)

Rest api
https://github.com/prince776/yt-projects/tree/master/cppREST

curl -X POST -F image=@image.jpg 'http://localhost:5000/v1/object-detection/yolov5'

- [ ] create docker files
- [ ] using COCO model in C++
- [ ] check C++ code deeper
- [ ] update presentation
  - [ ] endpoints
  - [ ] use case

# Docker

Build docker commands:

```bash
# build
docker build -t  yolov5-flask dockerfiles/restapi-python.Dockerfile
# run
docker run -p 5000:5000 yolov5-flask:latest
```





Run RestAPI client:

```
python request.py --image_path="../demo.jpg" --port=5000 --conf=0.4
```





docker run -it e8ee9478d3a5 bash
