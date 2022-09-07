#Program to perform background subtrction and cleanup the images
import numpy as np
from PIL import Image

#Helper funstion for the actual subtraction of two images, any pixel that
#is brighter in the bg than fg is set to 0 in the resulting pic
#Inputs are np arrays representing images
def subtractNoNeg(fg, bg):
    for i in range(0, len(fg)):
        for j in range(0, len(fg[i])):
            if(bg[i][j] > fg[i][j]):
                fg[i][j] = 0
            else:
                fg[i][j] = fg[i][j] - bg[i][j]
    return fg

#DoTheThing! 
#Inputs are file paths to the indended pics and filepath to where
#output should be written
def backgroundSub(fg, bg, out):
    fg = Image.open(fg)
    bg = Image.open(bg)
    if(fg.size != bg.size):
        print("Images are different sizes!")
        exit()
    fgnp = np.asarray(fg)
    bgnp = np.asarray(bg)
    fgnp = subtractNoNeg(fgnp, bgnp)
    Image.fromarray(fgnp).save(out)

#Driver code
#fg if the image you want to isolate the flakes in
#bg can be any other image from the same camera
fg = 'images/cam1/Flake000953_Cam1_1_2022-2-13-20-1-53-432.png'
bg = 'images/cam1/Flake000933_Cam1_1_2022-2-13-20-0-14-563.png'
out = 'images/cam1/output/output2.png'
backgroundSub(fg, bg, out)

