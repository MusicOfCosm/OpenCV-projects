# cython: cdivision=True
# cython: boundscheck=False
# cython: wraparound=False
# cython: language_level=3
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
# # cython: profile=True #This thing adds overhead to every function    python -m cProfile Actual_art.py

import cython
import cv2 as cv
import random
import numpy as np
cimport numpy as np
# np.import_array()
import array
# from cpython cimport array
# import cProfile
from libc.stdlib cimport rand, srand, RAND_MAX
from libc.string cimport strcmp
from libc.stdint cimport uint8_t
from libc.math cimport sin, cos
from libc.limits cimport INT_MAX
from libc.time cimport clock_t, clock, CLOCKS_PER_SEC, time
srand(time(NULL))

#For performance
cdef float pi = 3.141592653589

cdef int round(float i) nogil:
    cdef int o = <int>i
    if o + 1 - i < i - o:
        o +=  1
    return o


cdef double randomize() nogil: #Between 0 and 1
    return <double>rand() / RAND_MAX

cdef int randint(int lowest_val, int highest_val) nogil:
    cdef double rtrn_val = (highest_val - lowest_val) * randomize() + lowest_val
    return round(rtrn_val)

cdef float uniform(float lowest_val, float highest_val) nogil:
    return (highest_val - lowest_val) * randomize() + lowest_val


from sys import platform
if platform.startswith('win'):
    from ctypes import windll
    windll.shcore.SetProcessDpiAwareness(1)

import tkinter
root = tkinter.Tk()
cdef float diviser = 2
cdef int HEIGHT = root.winfo_screenwidth()//diviser
cdef int WIDTH = root.winfo_screenheight()//diviser
root.destroy()

cdef int NUMBER = 50000#int(10000//diviser**2)#((HEIGHT*WIDTH)//100)*randint(5, 10)
cdef float SPEED = uniform(1, 3)
cdef int SENSOR_RANGE = randint(5, 12)
cdef int SENSOR_SIZE = 0
cdef bint BOUNDARY = False#randint(0, 1) #0 and 1 are converted to False and True
choice = random.randint(0, 2)
cdef char *START
if choice == 0:
    START = 'random'
elif choice == 1:
    START = 'centre'
elif choice == 2:
    START = 'circle'
print(f'Number: {NUMBER}\nSpeed: {SPEED}\nSensor range: {SENSOR_RANGE}\nSensor size: {SENSOR_SIZE}\nBoundaries: {BOUNDARY}\nStart: {START}')

cdef int i #For for loops
cdef uint8_t channels, a, b, c
cdef uint8_t color[3]  #Unsigned 8 bit integer (0 to 255), literally one byte
cdef bint color_bool = random.randint(0, 1)

if color_bool:
    a = <uint8_t>randint(0, 255) 
    b = <uint8_t>randint(0, 255)
    c = <uint8_t>randint(0, 255) #85 is 255/3

else:
    a = 255
    b = 255
    c = 255
color[:] = [a, b, c]
channels = 3
print(f'Colored: B:{color[0]}, G:{color[1]}, R:{color[2]}')
cdef uint8_t gray_max = <uint8_t>((a+b+c)/3)


cdef int sense(float x, float y, float orientation, float sensorAngleOffset, np.uint8_t[:,::1] gray,
    bint BOUNDARY, int width, int height, int SENSOR_RANGE, int SENSOR_SIZE) nogil:
    cdef int somme, offsetX, offsetY, posX, posY
    cdef float sensorDir_X, sensorDir_Y, sensorCentre_X, sensorCentre_Y
    cdef double sensorAngle

    sensorAngle = orientation + sensorAngleOffset

    sensorDir_X, sensorDir_Y = cos(sensorAngle), sin(sensorAngle)
    sensorCentre_X = x + sensorDir_X * SENSOR_RANGE
    sensorCentre_Y = y + sensorDir_Y * SENSOR_RANGE
    somme = 0
    offsetX = -SENSOR_SIZE
    offsetY = -SENSOR_SIZE

    for offsetX in range(-SENSOR_SIZE, SENSOR_SIZE+1):
        for offsetY in range(-SENSOR_SIZE, SENSOR_SIZE+1):
            posX = round(sensorCentre_X + offsetX)
            posY = round(sensorCentre_Y + offsetY)
            posX = round(X_boundary(posX, BOUNDARY, width-1, height-1)) #boundary returns a float
            posY = round(Y_boundary(posY, BOUNDARY, width-1, height-1))
            
            if BOUNDARY is True:
                if posX >= 0 and posX < width and posY >= 0 and posY < height:
                    # somme += gray.item(posX, posY)
                    somme += gray[round(posX), round(posY)]
            else: 
                # somme += gray.item(posX, posY)
                somme += gray[round(posX), round(posY)]

    return somme


cdef float X_boundary(float x, bint BOUNDARY, int W, int H) nogil:
    if BOUNDARY == True:
        if x >= W:
            x = W
        if x < 0:
            x = 0
    else:
        if x > W:
            x -= W
        if x < 0:
            x += W
    return x
cdef float Y_boundary(float y, bint BOUNDARY, int W, int H) nogil:
    if BOUNDARY == True:
        if y >= H:
            y = <float>H
        if y < 0:
            y = 0
    else:
        if y > H:
            y -= <float>H
        if y < 0:
            y += H
    return y


cdef class Agents(object):
    cdef int width, height, forward, left, right
    cdef float x, y, orientation, SA, RA
    cdef float tmp_pos

    def __cinit__(object self, char *START, bint color_bool, int width, int height, float sensor_angle=pi/4, float rotation_angle=pi/3):
        '''Initializes a random orientation, as well as color'''
        
        self.width = width-1
        self.height = height-1
        self.forward = 0
        self.left = 0
        self.right = 0
        tmp_pos = uniform(-pi, pi)

        if strcmp(START, 'random') == 0:
            self.x = randint(0, self.width)
            self.y = randint(0, self.height)
        elif strcmp(START, 'centre') == 0:
            self.x = float(self.width//2)
            self.y = float(self.height//2)
        elif strcmp(START, 'circle') == 0:
            self.x = float(self.width //2) + cos(tmp_pos) * randint(0, height // 2)
            self.y = float(self.height//2) + sin(tmp_pos) * randint(0, height // 2)
        del tmp_pos

        self.orientation = randomize() * 2*pi
        self.SA = sensor_angle
        self.RA = rotation_angle

    cpdef void movement(self, uint8_t color[3], np.uint8_t[:,:,::1] canvas, np.uint8_t[:,::1] gray,
                        float SPEED, bint BOUNDARY, int SENSOR_RANGE, int SENSOR_SIZE):
        '''Orientation, interaction with or without other agents, and movement.'''
        
        with nogil:
            if gray[round(self.x)][round(self.y)] == gray_max:
                self.orientation = randomize() * 2*pi
            else:
                canvas[round(self.x)][round(self.y)][0] = color[0]
                canvas[round(self.x)][round(self.y)][1] = color[1]
                canvas[round(self.x)][round(self.y)][2] = color[2]

            self.forward = sense(self.x, self.y, self.orientation, 0.0, gray, BOUNDARY, self.width, self.height, SENSOR_RANGE, SENSOR_SIZE)
            self.left = sense(self.x, self.y, self.orientation, self.SA, gray, BOUNDARY, self.width, self.height, SENSOR_RANGE, SENSOR_SIZE)
            self.right = sense(self.x, self.y, self.orientation, -self.SA, gray, BOUNDARY, self.width, self.height, SENSOR_RANGE, SENSOR_SIZE)

            if self.forward > self.left and self.forward > self.right:
                pass
            
            # elif self.forward < self.left and self.forward < right:
            #     self.orientation += random.choice((self.RA, self.RA))
            
            elif self.left > self.right:
                self.orientation += self.RA
                
            elif self.left < self.right:
                self.orientation += -self.RA


            self.x += SPEED * cos(<double>self.orientation)
            self.y += SPEED * sin(<double>self.orientation)

            if BOUNDARY == True:
                if self.x >= self.width or self.x < 0 or self.y >= self.height or self.y < 0:
                    self.orientation = randomize() * 2*pi
            self.x = X_boundary(self.x, BOUNDARY, self.width, self.height)
            self.y = Y_boundary(self.y, BOUNDARY, self.width, self.height)


cdef int main():
    #Error: Array element cannot be a Python object
    # cdef object agents[NUMBER]
    # for i in range(NUMBER):
    #     agents[i] = Agents(START, color_bool, WIDTH, HEIGHT)
    cdef list agents = [Agents(START, color_bool, WIDTH, HEIGHT) for _ in range(NUMBER)]

    # cdef uint8_t [1080][1920][3] canvas
    cdef np.uint8_t[:,:,::1] canvas = np.zeros((WIDTH, HEIGHT, channels), dtype='uint8')
    cv.namedWindow('Canvas', cv.WINDOW_NORMAL)
    cv.setWindowProperty('Canvas', cv.WND_PROP_FULLSCREEN, cv.WINDOW_FULLSCREEN)
    cv.imshow('Canvas', np.asarray(canvas))

    img_map = cv.imread("Art_map.png", 0)
    img_map = cv.resize(img_map, (HEIGHT, WIDTH))
    # inverted = cv2.bitwise_not(img_map)

    cdef clock_t start, finish
    cdef int value = 1
    cdef np.ndarray[np.uint8_t, ndim=3] mat = np.ones((WIDTH, HEIGHT, channels), dtype=np.uint8)*value #How fast is the fade (it was 3)
    cdef np.uint8_t[:,::1] gray
    # cdef float value = 0.99
    cdef float mean = 0
    while True:
        start = clock()

        canvas = cv.subtract(np.asarray(canvas), mat)
        canvas = cv.subtract(np.asarray(canvas), mat)
        # canvas[:,:] = canvas*value #0.99

        canvas = cv.GaussianBlur(np.asarray(canvas), (3,3), 0)

        gray = cv.cvtColor(np.asarray(canvas), cv.COLOR_BGR2GRAY)

        #Comment to have it normal, uncomment to have it use the image
        gray = cv.subtract(img_map, np.asarray(gray))
        # gray = cv.add(np.asarray(gray), img_map)//127
        # gray = img_map

        for i in range(NUMBER):
            agents[i].movement(color, canvas, gray, SPEED, BOUNDARY, SENSOR_RANGE, SENSOR_SIZE)

        # img = numpy.array(sct.grab(monitor))
        cv.imshow('Canvas', np.asarray(canvas))

        finish = clock()
        if mean == 0:
            mean = <float>(finish - start) / CLOCKS_PER_SEC
        else:
            mean += <float>(finish - start) / CLOCKS_PER_SEC
            mean /= 2
        
        key = cv.pollKey()
        if key == 27: #escape
            break
        elif key == 32: #space
            while True: 
                key = cv.waitKey(1000)
                if key == 32:
                    break
    cv.destroyAllWindows()
    print(f'Average: {1/mean} fps') #{mean} seconds per frame

main()