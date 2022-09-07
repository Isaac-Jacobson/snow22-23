#Python program for pre-processing images before labeling 
#Three inputs, two images taken from the same camera angle and
#a string path to where the output should be written
#Cleans (background subtraction+noise removal) the image then 
#crops individual snowflakes in a 300*300 image and saves them

import sys
from xml.etree.ElementTree import tostring
import numpy as np
from PIL import Image, ImageFilter
import cv2 as cv
import random
import re
from copy import deepcopy

#Helper function for the actual subtraction of two images, any pixel that
#is brighter in the bg than fg is set to 0 in the resulting pic
#Inputs are np arrays representing images
def subtractNoNeg(fg, bg, threshold):
    fg = np.subtract(fg, bg)

def subtractNoNeg1(fg, bg, threshold):
    for i in range(0, len(fg)):
        for j in range(0, len(fg[i])):
            if(bg[i][j] < threshold):
                if(bg[i][j] > fg[i][j]): fg[i][j] = 0
                else: fg[i][j] = fg[i][j] - bg[i][j]
    return fg

#Like the above function but only for images from cam0
#Removes subtraction threshold for bottom left corner of image
#this allows the flash to be removed but introduces error if 
#snowflakes in the two images overlap in the corners of the flash rectangle
def subtractCam0(fg, bg, threshold):
    #flash starts at (0, 825) to (0, 1650) down and (400, 1235) out
    for i in range(0, len(fg)):
        for j in range(0, len(fg[i])):
            if((i < 400 and j > 825 and j < 1650) or bg[i][j] < threshold):
                if(bg[i][j] > fg[i][j]): fg[i][j] = 0
                else: fg[i][j] = fg[i][j] - bg[i][j]
    return fg

#Inputs are file paths to the indended pics and threshold for
#not subtracting (overlapped flakes avoidance)
def backgroundSub(fg, bg, threshold=100):
    fgPic = Image.open(fg)
    bgPic = Image.open(bg)
    if(fgPic.size != bgPic.size):
        print("Images are different sizes! Exiting!")
        exit()
    fgnp = np.asarray(fgPic)
    bgnp = np.asarray(bgPic)
    if('Cam0' in fg):fgnp = subtractCam0(fgnp, bgnp, threshold)
    else: fgnp = subtractNoNeg(fgnp, bgnp, threshold)
    #Image.fromarray(fgnp).save(out)
    return fgnp

#Add noise to image, used for testing
#Hard coded thresholds!!!
def addNoise(img, out):
    odds = 100
    intensityLower = 100
    pic = Image.open(img)
    pic = np.asarray(pic)
    for i in range(0, len(pic)-1):
        for j in range(0, len(pic[i])-1):
            if(pic[i][j] < 100):
                num = random.randrange(0, odds, 1)
                if(num == 1): 
                    pic[i][j] = random.randrange(intensityLower, 255, 1)
    Image.fromarray(pic).save(out)

#Removes small clusters of bright particles, used to remove noise
#Hard coded thresholds!!!
def guassianBlur(img):
    pic = Image.fromarray(img)
    blur = pic.filter(ImageFilter.GaussianBlur(radius=4))
    blur = np.asarray(blur)
    ret,bin = cv.threshold(blur,40,255,cv.THRESH_BINARY)
    #Image.fromarray(bin).save(out)
    return bin

#Takes the original image and the blurred image
#Reconstructs the inside details of a blurred snowflake
def sharpen(img, imgBlur):
    for i in range(0, len(img)):
        for j in range(0, len(img[i])):
            if(imgBlur[i][j]==255):
                img[i][j] = img[i][j]
            else:
                img[i][j] = 0
    #Image.fromarray(img).save(out)
    return img

#Finds contours in the image, not used!!!
#Hardcoded thresholds!!!
def findContour(img, out):
    pic = Image.open(img)
    pic = np.asarray(pic)
    ret,bin = cv.threshold(pic,50,255,cv.THRESH_BINARY)
    contours, hierarchy = cv.findContours(bin, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    img_contours = np.zeros(pic.shape)
    cv.drawContours(img_contours, contours, -1, 255)
    cv.imwrite(out,img_contours) 

#Saves the image to the output path
#Num is the number of the flake from the original image
def save(img, outPath, num, filename):
    path = outPath + filename[12:len(filename)-4] + "Cropped" + str(num) + ".png"
    print("Image saved to: " + path + ": " + str(cv.imwrite(path, img)))
    #print(cv.imwrite(path, img))

#Crops each individual snowflake from an image and saves 
#to the output path
#Lots of hardcoded thresholds!!!!
#*****Pulled from an old clab notebook*****
def cropIndividualFlakes(imgCpy, out, filename):
    stepSize = 400
    stepSize2 = 20
    threshold1 = 40 
    threshold2 = 150
    threshold3 = 40

    dimensionX = 300
    dimensionY = 300

    img = deepcopy(imgCpy)
    hist = cv.calcHist([imgCpy],[0],None,[256],[0,256])
    hist = np.asarray(hist).flatten()

    ret,img2 = cv.threshold(img,threshold1,255,cv.THRESH_TOZERO)
    
    # Use "max pooling" (If any pixels in window are high intensity, bring them all up to high intensity) to try to ensure box captures entire snowflake 
    for i in range(0, len(img2)-stepSize2, stepSize2):
        for j in range(0, len(img2[i])-stepSize2, stepSize2):
            sum = 0
            for x in range(stepSize2):
                for y in range(stepSize2):
                    sum += img2[i+x][j+y]
            if sum >= (stepSize2**2) * threshold3:
                for x in range(stepSize2):
                    for y in range(stepSize2):
                        img2[i+x][j+y] = 255
            
    edges = cv.Canny(image=img2, threshold1=0, threshold2=250)
    
    cnts = cv.findContours(edges, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE)
    cnts = cnts[0] if len(cnts) == 2 else cnts[1]
    count = 0
    for c in cnts:
        x,y,w,h = cv.boundingRect(c)
                
        middleX = int(x + w/2)
        middleY = int(y + h/2)
            
        # Naive case, where snowflake is in center of image
        minX = middleX-int(dimensionX/2)
        minY = middleY-int(dimensionY/2)
            
        #Test to make sure dimensions won't be out of bounds
        if minX < 0:
            minX = 0
        if minY < 0:
            minY = 0
        if minX + dimensionX > len(img[0]):
            minX = len(img[0])-dimensionX-1
        if minY + dimensionY > len(img):
            minY = len(img)-dimensionY-1
            
        imgCropped = imgCpy[minY:minY + dimensionY, minX:minX+dimensionX]
        imgBoxed = deepcopy(img)
        cv.rectangle(imgBoxed, (x, y), (x + w, y + h), (36,255,12), 2)
            
        hist2 = cv.calcHist([imgCpy],[0],None,[256],[0,256])
        hist2 = np.asarray(hist2).flatten()
            
        save(imgCropped, out, count, filename)
        count+=1

def cropFlakes(img, imgC):
    #thresh = cv.fromarray(img)    
    # find contours in the binary image
    im2, contours = cv.findContours(img,cv.RETR_TREE,cv.CHAIN_APPROX_SIMPLE)
    for c in contours:
    # calculate moments for each contour
        # M = cv.moments(c)

        # # calculate x,y coordinate of center
        # cX = int(M["m10"] / M["m00"])
        # cY = int(M["m01"] / M["m00"])
        # cv.circle(img, (cX, cY), 5, (255, 255, 255), -1)
        # cv.putText(img, "centroid", (cX - 25, cY - 25),cv.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
        print("found a flake")
    

#Zhu Li do the thing!
def zhuLi(fg, bg, out):
    #Background subtraction
    #Noise cleanup
        #Blur
        #sharpen
    #crop
        #save each
    arr = backgroundSub(fg, bg)
    blurArr = guassianBlur(arr)
    arr = sharpen(arr, blurArr)
    cropIndividualFlakes(arr, out, fg)

#Driver code
#fg if the image you want to isolate the flakes in
#bg can be any other image from the same camera
if(len(sys.argv) != 4):
    print("Using hard-coded values instead of cmd line args")
    fg = 'images/cam1/Flake000953_Cam1_1_2022-2-13-20-1-53-432.png'
    bg = 'images/cam1/Flake000933_Cam1_1_2022-2-13-20-0-14-563.png'
    out = 'images/cam1/output/'
else: 
    fg = sys.argv[1]
    bg = sys.argv[2]
    out = sys.argv[3]
    print("Image: " + fg)
image = Image.open("images/cam1/output/output1Blur .png")
image = np.asarray(image)
imageC = 0
cropFlakes(image, imageC)
#zhuLi(fg, bg, out)
