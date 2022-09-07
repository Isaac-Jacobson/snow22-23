import sys
from xml.etree.ElementTree import tostring
import numpy as np
from PIL import Image, ImageFilter
import cv2 as cv
from copy import deepcopy
import imutils

#Takes 3 inputs, 2 image path and path to output folder

#Takes 2 images and an output path
def doTheThing(fg, bg, out):
    filename = fg
    #Load images as arrays
    fg=np.asarray(Image.open(fg))
    bg=np.asarray(Image.open(bg))

    #Straight up background subtraction
    img = cv.subtract(fg, bg)

    #Remove negative values in new image
    img = np.maximum(0, img)

    #Blur to remove noise
    pic = Image.fromarray(img)
    blur = pic.filter(ImageFilter.GaussianBlur(radius=25))
    blur = np.asarray(blur)

    #Binarize the image
    ret,bin = cv.threshold(blur,25,255,cv.THRESH_BINARY)

    #Find contours in binarized image
    contours = cv.findContours(bin, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    contours = imutils.grab_contours(contours)

    num = 0
    #For each contour, find the center and crop it
    for flake in contours:  
        x,y,w,h = cv.boundingRect(flake)
        cX = int(x + w/2)
        cY = int(y + h/2)

        crop = np.concatenate((np.zeros((len(fg), 150)), fg, np.zeros((len(fg), 150))), axis = 1)
        crop = np.concatenate((np.zeros((150, len(crop[0]))), crop, np.zeros((150, len(crop[0])))), axis = 0)

        flakeImg = crop[cY:cY+300, cX:cX+300]
        flakeImg = np.asarray(flakeImg)

        ret,fin = cv.threshold(flakeImg,45,255,cv.THRESH_TOZERO)
        
        path = out + filename[12:len(filename)-4] + "Cropped" + str(num) + ".png"
        print("Image saved to: " + path + ": " + str(cv.imwrite(path, fin)))
        num += 1

#Driver code
if(len(sys.argv) != 4):
    print("Issue with number of inputs")
else: 
    fg = sys.argv[1]
    bg = sys.argv[2]
    out = sys.argv[3]
doTheThing(fg, bg, out)