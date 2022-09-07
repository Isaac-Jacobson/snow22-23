import cv2
import numpy as np
import pandas as pd
#import tkinter
import matplotlib
#matplotlib.use('TkAgg')
import matplotlib.pyplot as plt 
from skimage.io import imread, imshow
from skimage.filters import gaussian, threshold_otsu
from skimage import measure
from sklearn import preprocessing
import math

bg1 = imread('bg_1.bmp')
bg2 = imread('bg_2.bmp')
flake1 = imread('flake1_1.bmp')
flake2 = imread('flake1_2.bmp')

#imshow(bg1)
#imshow(bg2)
#imshow(flake1)
#imshow(flake2)

#plt.savefig("output.png")
#cv2.waitKey(0)

blurred1 = gaussian(flake1, sigma=0.4)
bgRemd1 = blurred1 > threshold_otsu(blurred1)
blurred2 = gaussian(flake2, sigma=0.13)
bgRemd2 = blurred2 > threshold_otsu(blurred2)


#imshow(bgRemd1)
plt.imshow(bgRemd1)


#imshow(bgRemd2)
plt.show(bgRemd2)
#cv2.waitKey(0)


# props = measure.regionprops(bgRemd1)
# allCentroids = [props.Centroid]
# allArea1 = [props.Area]
# xCentroids = allCentroids(1:2:end)
# keepers1 = (allArea1 > 200)
# indexes1 = np.where(keepers1 > 0)
# radius = math.sqrt(allArea1/math.pi)

# bgRemd1 = ismember(labeledImage, indexes1);
# edge1 = edge(bgRemd1);
# points = edge1(edge1==)
# %centroid detection
# xsum = 0;
# ysum = 0;
# count = 0;
# for x = 1:1200
#     for y = 1:1920
#         if(bgRemd1(x,y) == 1)
#             xsum = xsum+x;
#             ysum = ysum+y;
#             count = count+1;          
#         end
#     end
# end
# y1 = round(xsum/count);
# x1 = round(ysum/count);


# figure,imshow(edge1); 
# hold on
# plot(x1,y1,'rx');
# axis on
# hold off
# % figure,imshow(edge2);

# labeledImage = bwlabel(bgRemd2);
# props = regionprops(labeledImage, 'Centroid', 'Area');
# allCentroids = [props.Centroid];
# allArea = [props.Area];
# xCentroids = allCentroids(1:2:end);
# keepers = (allArea > 200);
# indexes = find(keepers);

# bgRemd2 = ismember(labeledImage, indexes);
# edge2 = edge(bgRemd2);
# % blobsToExclude = ismember(labeledImage, indexes);
# % Erase those blobs from our original binary image.
# % bgRemd2(blobsToExclude) = false; 
# % Re-measure what's left.
# % labeledImage = bwlabel(bgRemd2);
# % props = regionprops(labeledImage, 'Centroid', 'Area');

# figure,imshow(bgRemd2);
# %hough transform
# [H,T,R] = hough(bgRemd2);
# P = houghpeaks(H,5,'threshold',ceil(0.3*max(H(:))));
# x = T(P(:,2)); y = R(P(:,1));
# lines = houghlines(bgRemd2,T,R,P,'FillGap',5,'MinLength',7);

# max_len = 0;
# sum1x = 0;
# sum1y = 0;
# sum2x = 0;
# sum2y = 0;

# figure, imshow(edge1+edge2), hold on
# for k = 1:length(lines)
#    xy = [lines(k).point1; lines(k).point2];
# %    plot(xy(:,1),xy(:,2),'LineWidth',2,'Color','green');
# % 
# % %    Plot beginnings and ends of lines
# %    plot(xy(1,1),xy(1,2),'x','LineWidth',2,'Color','yellow');
# %    plot(xy(2,1),xy(2,2),'x','LineWidth',2,'Color','red');
#    sum1x = sum1x+xy(1,1);
#    sum1y = sum1y+xy(1,2);
#    sum2x = sum2x+xy(2,1);
#    sum2y = sum2y+xy(2,2);
#    % Determine the endpoints of the longest line segment
# %    len = norm(lines(k).point1 - lines(k).point2);
# %    if ( len > max_len)
# %       max_len = len;
# %       xy_long = xy;
# %    end
# end
# avg1x = round(sum1x/length(lines));
# avg1y = round(sum1y/length(lines));
# avg2x =  round(sum2x/length(lines));
# avg2y =  round(sum2y/length(lines));

# d2 = avg2y - avg1y;
# dx2 = avg2x - avg1x;
# angle2 = atan(d2/dx2);

# ry = radius*sin(angle2);
# rx = radius*cos(angle2);

# avg1x = avg1x+rx;
# avg1y = avg1y+ry;

# avg2x = avg2x-rx;
# avg2y = avg2y-ry;

#    zz = [avg1x avg1y;avg2x avg2y];
   
#     plot(x1,y1,'rx');
#    plot(zz(:,1),zz(:,2),'LineWidth',2,'Color','blue');
#    plot(avg1x,avg1y,'x','LineWidth',2,'Color','yellow');
#    plot(avg2x,avg2y,'x','LineWidth',2,'Color','red');
# plot(x1,y1-radius,'x','LineWidth',2,'Color','red');
# plot(x1,y1+radius,'x','LineWidth',2,'Color','red');
# d1 = avg1y - y1
# dx1 = avg1x - x1;

# angle1 = atan(d1/dx1);
# d2 = avg2y - avg1y
# dx2 = avg2x - avg1x;
# toc
# speed1 = d1/0.027
# speed2 = d2/0.021
