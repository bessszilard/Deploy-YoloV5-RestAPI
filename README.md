# Qt-deploy-YoloV5

Deploy YoloV5

## Used repository

- [GitHub - ultralytics/yolov5: YOLOv5 🚀 in PyTorch &gt; ONNX &gt; CoreML &gt; TFLite](https://github.com/ultralytics/yolov5)

- [GitHub - freshtechyy/ONNX-Runtime-GPU-image-classifciation-example](https://github.com/freshtechyy/ONNX-Runtime-GPU-image-classifciation-example)

## Tutorials

Python yolov5 flask docker file

[yolov5-flask/Dockerfile at master · robmarkcole/yolov5-flask · GitHub](https://github.com/robmarkcole/yolov5-flask/blob/master/Dockerfile)

Rest api
https://github.com/prince776/yt-projects/tree/master/cppREST

curl -X POST -F image=@image.jpg 'http://localhost:5000/v1/object-detection/yolov5'

- [x] create docker files
- [ ] using COCO model in C++
- [ ] check C++ code deeper
- [x] update presentation
  - [x] endpoints
  - [x] use case

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
python restapi-client.py --image_path="../demo.jpg" --port=5000 --conf=0.4
```

```

```

Install NVidia toolkit

```
sudo apt-get install -y nvidia-container-toolkit
```

## CPP docker TODO

- Add `sudo apt install nvidia-cuda-toolkit`

Export .pt file to .onnx
https://colab.research.google.com/drive/1V-F3erKkPun-vNn28BoOc6ENKmfo8kDh?usp=sharing#scrollTo=12t-V70DmpSl

```
cmake .. && make && ./src/main --model_path ../yolov5s6.onnx --class_names ../coco.names --gpu --port 3000
```
