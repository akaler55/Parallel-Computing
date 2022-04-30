# -*- coding: utf-8 -*-
"""
Created on Thu Apr 28 12:15:48 2022

@author: Amandeep Singh
"""
import cv2
import os
#Read all frames
image_folder = 'tmp3'
#Resulting video
video_name = 'results//output_video.mp4'

#images = [img for img in os.listdir(image_folder) if img.endswith(".bmp")]
images = []
for i in range(1,144):
    images.append("image"+str(i+1)+".bmp")


frame = cv2.imread(os.path.join(image_folder, images[0]))
height, width, layers = frame.shape
#convert frames to video
video = cv2.VideoWriter(video_name, 0, fps = 15, frameSize = (width,height))

for image in images:
    video.write(cv2.imread(os.path.join(image_folder, image)))

cv2.destroyAllWindows()
video.release()