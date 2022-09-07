#Program for performing background subtraction on images
#Takes 2 inputs from the comand line, path to image (fg) and a path to second image (bg)
import sys
import numpy as np
from PIL import Image, ImageFilter
import cv2 as cv
import copy

from pandas import Int8Dtype

def threshold(thresh, bg):
    for r in range(0,len(bg)):
        bg[r] = (bg[r] < thresh) * bg[r]
    return bg

def threshold1(thresh, bg):
    for r in range(0,len(bg)):
        bg[r] = (bg[r] > thresh) * bg[r]
    return bg

def thresholdCam0(thresh, bg):
    for r in range(0,725):
        bg[r] = (bg[r] < thresh) * bg[r]
    for r in range(725, 1725):
        tmp = (bg[r] < thresh) * bg[r]
        bg[r][500:] = tmp[500:]
    for r in range(1725,len(bg)):
        bg[r] = (bg[r] < thresh) * bg[r]
    return bg

def doTheThing(fg, bg, out):
    fg=np.asarray(Image.open(fg))
    bg=np.asarray(Image.open(bg))
    thresh = 80 #THRESHOLD!!!
    if('Cam0' in out): bg = thresholdCam0(thresh, bg)
    else: bg = threshold(thresh, bg)
    img = cv.subtract(fg, bg)
    img = np.maximum(0, img)
    img = threshold1(20, img)
    pic = Image.fromarray(img)
    blur = pic.filter(ImageFilter.GaussianBlur(radius=4))
    blur = np.asarray(blur)
    ret,bin = cv.threshold(blur,15,255,cv.THRESH_BINARY)
    img = (bin == 255) * img
    Image.fromarray(img).save(out)
    print("Image saved to: " + out)

#Driver code
if(len(sys.argv) != 3):
    print("Issue with number of inputs")
else: 
    fg = sys.argv[1]
    bg = sys.argv[2]
out = fg[:len(fg)-4] + 'NoBackground.png'
doTheThing(fg, bg, out)
