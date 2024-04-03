import numpy as np
import cv2

import os

import random

print(cv2.__version__)
# version 4.9.0

CUR_DIR = os.path.abspath('.')
print(CUR_DIR)

image = cv2.imread(CUR_DIR+"/assets/depth/1341836561.612401.png")
print(image.shape)

idx = 0

thr = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

for x in range(0,thr.shape[0]):
    for y in range(0,thr.shape[1]):
        if thr[x][y]>38:
            thr[x][y]=255

coutours, hier = cv2.findContours(thr,cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
  
while idx >= 0:
    # 랜덤 색상 지정
    c = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
    
    # 외곽선 그리기
    # hier 계층 정보를 입력했으므로 모든 외곽선에 그림을 그리게 됩니다.
    cv2.drawContours(image, coutours, idx, c, 2, cv2.LINE_8, hier)
    
    # 0번째 계층만 그리기. 하지만 hier 계층 정보를 입력했기 때문에 모든 외곽선에 그림을 그립니다.
    # 계층 정보를 입력 안하면 0번 계층만 그립니다.
    idx = hier[0, idx, 0]

cv2.imshow("Moon",image)
cv2.waitKey(0)




# cap = cv2.VideoCapture(CUR_DIR+'/assets/exmapleObstacle.MOV')

# while(cap.isOpened()):
#     ret, frame = cap.read()
#     if(ret):
#         print(ret)
#         gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
#         cv2.imshow('frame',gray)
#     else:
#         break

#     if cv2.waitKey(1) & 0xFF == ord('q'):
#         break
# cap.release()
# cv2.destroyAllWindows()