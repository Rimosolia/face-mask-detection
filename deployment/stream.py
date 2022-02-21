import pandas as pd
import cv2
import urllib.request
import numpy as np
import os
from datetime import datetime
from detect1 import *
 
path = r'E:/Process/'
url='http://192.168.1.9/cam-mid.jpg'
 
 
while True:
    cap = cv2.VideoCapture(url)
    ret, img = cap.read()
    if ret == True:
        cv2.imwrite("E:/Process/Frame.jpg", img)
    else:
        break
    try:
        temp = str(detect(save_img = True, out = "static/image" ,source = "E:/Process/Frame.jpg"))
    except:
        temp = 'NoMask 1.0'
    img= cv2.imread('E:/Working/mask-detector-master/mask-detector-master/deployment/static/image/test.jpg')
    cv2.imshow('Live', img)
    key=cv2.waitKey(5)
    if key==ord('q'):
        break

    number = float(temp.split()[1])
    result = temp.split()[0]

    if result == "NoMask":
        if number > 0.6:
            print("Không đeo khẩu trang ")
        else:
            print("Đã đeo khẩu trang")
    else:
        if number > 0.6:
            print("Đã đeo khẩu trang ")
        else:
            print("Không đeo đeo khẩu trang")
cv2.destroyAllWindows()
cv2.imread