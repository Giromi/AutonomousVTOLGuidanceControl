#!/usr/bin/python
import socket
import cv2
import numpy

#연결할 서버(수신단)의 ip주소와 port번호
TCP_IP = 'localhost'
TCP_PORT = 5001

#송신을 위한 socket 준비
sock = socket.socket()
sock.connect((TCP_IP, TCP_PORT))

#OpenCV를 이용해서 webcam으로 부터 이미지 추출
capture = cv2.VideoCapture(0)
ret, frame = capture.read()

#추출한 이미지를 String 형태로 변환(인코딩)시키는 과정
encode_param=[int(cv2.IMWRITE_JPEG_QUALITY),90]
result, imgencode = cv2.imencode('.jpg', frame, encode_param)
data = numpy.array(imgencode)
stringData = data.tostring()

#String 형태로 변환한 이미지를 socket을 통해서 전송
sock.send( str(len(stringData)).ljust(16));
sock.send( stringData );
sock.close()

#다시 이미지로 디코딩해서 화면에 출력. 그리고 종료
decimg=cv2.imdecode(data,1)
cv2.imshow('CLIENT',decimg)
cv2.waitKey(0)
cv2.destroyAllWindows() 