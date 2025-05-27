import cv2
import numpy as np
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 9600) 
time.sleep(2)  

image = cv2.imread("input.jpg") 

hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

lower_blue = np.array([0, 0, 200])
upper_blue = np.array([180, 50, 255])

mask1 = cv2.inRange(hsv, lower_blue, upper_blue)

image2 = cv2.bitwise_and(image, image, mask=mask1)

x = 0
y = 0
count = 0
for i in range(len(image)):
    for j in range(len(image[i])):
        if image2[i][j][2]:
            x += i
            y += j
            count += 1
x = x/count/len(image2)*20
y = y/count/len(image2[0])*20
input()
ser.write((str(x) + " " + str(y) + "\n").encode())
print("Sent:", x, y, "\n")
  

# ser.close()

# Optional display (small size)
resized = cv2.resize(image2, None, fx=0.2, fy=0.2)
cv2.imshow("Result", resized)
cv2.waitKey(0)
cv2.destroyAllWindows()
