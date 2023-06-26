import requests
import cv2
import numpy as np
import pandas as pd
import argparse

# source https://inside-machinelearning.com/en/bounding-boxes-python-function/
def box_label(image, box, label='', color=(128, 128, 128), txt_color=(255, 255, 255)):
  lw = max(round(sum(image.shape) / 2 * 0.003), 2)
  p1, p2 = (int(box[0]), int(box[1])), (int(box[2]), int(box[3]))
  cv2.rectangle(image, p1, p2, color, thickness=lw, lineType=cv2.LINE_AA)
  if label:
    tf = max(lw - 1, 1)  # font thickness
    w, h = cv2.getTextSize(label, 0, fontScale=lw / 3, thickness=tf)[0]  # text width, height
    outside = p1[1] - h >= 3
    p2 = p1[0] + w, p1[1] - h - 3 if outside else p1[1] + h + 3
    cv2.rectangle(image, p1, p2, color, -1, cv2.LINE_AA)  # filled
    cv2.putText(image,
                label, (p1[0], p1[1] - 2 if outside else p1[1] + h + 2),
                0,
                lw / 3,
                txt_color,
                thickness=tf,
                lineType=cv2.LINE_AA)

def plot_bboxes(image, predictions, conf, colors=[]):
  #Define colors
  if colors == []:
    colors = [(89, 161, 197),(67, 161, 255),(19, 222, 24),(186, 55, 2),(167, 146, 11),(190, 76, 98),(130, 172, 179),(115, 209, 128),(204, 79, 135),(136, 126, 185),(209, 213, 45),(44, 52, 10),(101, 158, 121),(179, 124, 12),(25, 33, 189),(45, 115, 11),(73, 197, 184),(62, 225, 221),(32, 46, 52),(20, 165, 16),(54, 15, 57),(12, 150, 9),(10, 46, 99),(94, 89, 46),(48, 37, 106),(42, 10, 96),(7, 164, 128),(98, 213, 120),(40, 5, 219),(54, 25, 150),(251, 74, 172),(0, 236, 196),(21, 104, 190),(226, 74, 232),(120, 67, 25),(191, 106, 197),(8, 15, 134),(21, 2, 1),(142, 63, 109),(133, 148, 146),(187, 77, 253),(155, 22, 122),(218, 130, 77),(164, 102, 79),(43, 152, 125),(185, 124, 151),(95, 159, 238),(128, 89, 85),(228, 6, 60),(6, 41, 210),(11, 1, 133),(30, 96, 58),(230, 136, 109),(126, 45, 174),(164, 63, 165),(32, 111, 29),(232, 40, 70),(55, 31, 198),(148, 211, 129),(10, 186, 211),(181, 201, 94),(55, 35, 92),(129, 140, 233),(70, 250, 116),(61, 209, 152),(216, 21, 138),(100, 0, 176),(3, 42, 70),(151, 13, 44),(216, 102, 88),(125, 216, 93),(171, 236, 47),(253, 127, 103),(205, 137, 244),(193, 137, 224),(36, 152, 214),(17, 50, 238),(154, 165, 67),(114, 129, 60),(119, 24, 48),(73, 8, 110)]
  
  #plot each boxes
  for index, pred in predictions.iterrows():
    box = [pred.xmin, pred.ymin, pred.xmax, pred.ymax]    
    
    label = f"{pred.confidence:.2f} {pred['name']})"
    if pred.confidence > conf:
        color = colors[pred['class']]
        box_label(image, box, label, color)
  #show image
  print('image saved to ')
  cv2.imwrite('../proc_image.png', image)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Sending rest api request")
    parser.add_argument("--port", default=5000, type=int, help="port number")
    parser.add_argument('--image_path', default='../image.jpg', help='image path')
    parser.add_argument('--verbose', default=0, type=int, help='debug')
    parser.add_argument('--conf', default=0.0, type=float, help='confidence threshold')
    args = parser.parse_args()

    image_path = args.image_path # '../image.jpg'
    # image_path = '../demo.jpg'

    f = open(image_path, 'rb')
    files = {"image": ("@image.jpg", f)}
    response = requests.post(f"http://127.0.0.1:{args.port}/v1/object-detection/yolov5", files=files)

    verbos = args.verbose

    try:
        if verbos:
            print(response.text)
        
        # TODOsz add error handling
        results = pd.read_json(response.text)
        image = cv2.imread(image_path)
        plot_bboxes(image, results, conf=args.conf)
        print(results[results.confidence > args.conf])
        
    except Exception as e:
        print(f"Failed to process the image: {str(e)}")