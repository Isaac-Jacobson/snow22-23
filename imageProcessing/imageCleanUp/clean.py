import numpy as np
from PIL import Image, ImageFilter
import sys
import cv2 as cv
import random

#Add noise to image, used for testing
odds = 100
intensityLower = 100
def addNoise(img, out):
    pic = Image.open(img)
    pic = np.asarray(pic)
    for i in range(0, len(pic)-1):
        for j in range(0, len(pic[i])-1):
            if(pic[i][j] < 100):
                num = random.randrange(0, odds, 1)
                if(num == 1): 
                    pic[i][j] = random.randrange(intensityLower, 255, 1)
    Image.fromarray(pic).save(out)

def guassianBlur(img, out):
    pic = Image.open(img)
    blur = pic.filter(ImageFilter.GaussianBlur(radius=4))
    blur = np.asarray(blur)
    ret,bin = cv.threshold(blur,40,255,cv.THRESH_BINARY)
    Image.fromarray(bin).save(out)

def sharpen(img, imgBlur, out):
    pic = Image.open(img)
    picBlur = Image.open(imgBlur)
    pic = np.asarray(pic)
    picBlur = np.asarray(picBlur)
    for i in range(0, len(pic)):
        for j in range(0, len(pic[i])):
            if(picBlur[i][j]==255):
                pic[i][j] = pic[i][j]
            else:
                pic[i][j] = 0
    Image.fromarray(pic).save(out)

def findContour(img, out):
    pic = Image.open(img)
    pic = np.asarray(pic)
    ret,bin = cv.threshold(pic,50,255,cv.THRESH_BINARY)
    contours, hierarchy = cv.findContours(bin, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    img_contours = np.zeros(pic.shape)
    cv.drawContours(img_contours, contours, -1, 255)
    cv.imwrite(out,img_contours) 

img = 'images/cam1/output/output1Noise.png'
imgBlur = 'images/cam1/output/output1Blur.png'
out = 'images/cam1/output/output1ReSharpen.png'

sharpen(img, imgBlur, out)
