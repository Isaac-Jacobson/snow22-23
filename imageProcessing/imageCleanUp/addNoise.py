import numpy as np
from PIL import Image
import sys
import random

odds = 100
intensityLower = 100

#Add noise to image, used for testing
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

img = 'images/cam1/output/output1.png'
out = 'images/cam1/output/output1Noise.png'

addNoise(img, out)
