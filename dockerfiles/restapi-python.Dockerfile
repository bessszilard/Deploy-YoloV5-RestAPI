FROM python:3.8-slim-buster

RUN apt-get update
RUN apt-get install ffmpeg libsm6 libxext6  -y

WORKDIR /app
ADD . /app
RUN pip3 install -r requirements.txt

EXPOSE 5000

CMD ["python3", "scripts/restapi.py", "--port=5000"]
