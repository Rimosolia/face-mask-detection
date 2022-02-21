import json
import argparse
from flask import Flask, jsonify, request, redirect, url_for, render_template, flash, Response
from werkzeug.utils import secure_filename
import os
import numpy as np
import cv2
# from predict_bdr import *
import time
from datetime import timedelta
from detect1 import *
import time
import re
import threading
import urllib.request

app = Flask(__name__)

UPLOAD_FOLDER = 'upload'  # uploading dir
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif'}  # legal file type

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['SEND_FILE_MAX_AGE_DEFAULT'] = timedelta(seconds=5)
print(app.config['SEND_FILE_MAX_AGE_DEFAULT'])
def background():
    global result
    global url
    path = r'E:/Process/'
    print("Connected with {}".format(url))
    while True:
        if (result < 5):
            cap = cv2.VideoCapture(url)
            ret, img = cap.read()
            cv2.imwrite("E:/Process/Frame.jpg", img)
            try:
                temp = str(detect(save_img = True, out = "static/images" ,source = "E:/Process/Frame.jpg"))
            except:
                temp = 'NoMask 1.0'
            number = float(temp.split()[1])
            result1 = temp.split()[0]

            if result1 == "NoMask":
                if number < 0.8:
                    result= result + 1
            else:
                if number > 0.8:
                    result = result + 1
        else:
            break
        print("{}".format(result))
        time.sleep(2)

def camera_stream():
    global result
    while True:
        cap = cv2.VideoCapture(url)
        ret, img = cap.read()     
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        cv2.imshow('Live', gray)
        key=cv2.waitKey(5)
        if key==ord('q'):
            break
        if (result == 5):
            break

@app.route('/First')
def First():
    temp= str(detect(save_img = True, out = "static/images", source = 'E:\Working\mask-detector-master\deployment\static\Face-Masks.jpg'))
    return "1"
    

@app.route('/Json', methods= ['POST'])
def Json():
    req_data = request.get_json()
    global url  
    url = req_data['LocalIP']
    url = url + "/cam-mid.jpg"
    final = "http://" + url
    url = final
    threading.Thread(target=background).start()
    threading.Thread(target=camera_stream).start()
    return "1"


@app.route('/Result')
def Result():
    global result
    if (result >= 5):
        return "1"
    else:
        return "0"




if __name__ == '__main__':
    global result
    result = 0
    app.run(host='0.0.0.0', port=2222)
