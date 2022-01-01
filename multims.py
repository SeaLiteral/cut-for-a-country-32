import os
import math

pagesHorizontally = 16
pagesVertically = 2
pagesUsed = [math.ceil(320/64), math.ceil(320/64)]

def img2tim (fileIn, fileOut, imgX, imgY, clutX, clutY, bpp):
    for i in (fileIn, fileOut): # Check filenames so they don't contain things like ampersands or backticks
        if not (isalnum(i.replace('_'))):
           raise ValueError ('Filename might be invalid: '+str(i))
    os.system ('img2tim -org '+str(int(imgX))+' '+str(int(imgY))+' -plt '+str(int(clueX))+' '+str(int(clueY))+' -o '+fileOut+' '+fileIn+' -bpp '+str(bpp))

imageSizes = [('texture64.png', 64, 64, 4),
              ('three-languages.png', 64, 64, 4)
              ]

pageWidth = 16*64 # bits

for iSize in imageSizes:
    imgWidth = iSize[1]*iSize[3]
    
