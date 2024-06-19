# !/usr/bin/env python3
import cv2 as cv #OpenCV COLORs work with bgr instead of rgb
import numpy as np
#import numba as nb # If a problem arises, check "C:\Users\...\AppData\Local\Programs\Python\Python310\Lib\site-packages\numba\__init__.py" line 139
import random
from math import cos, sin, pi
import time
# print(help(cv.subtract))
import sys

from sys import platform
if platform.startswith('win'): #if the program is being run on windows, the tkinter blurriness is removed
    from ctypes import windll
    windll.shcore.SetProcessDpiAwareness(1)
# print(help(windll.shcore.SetProcessDpiAwareness))
import tkinter
root = tkinter.Tk()
HEIGHT = round(root.winfo_screenwidth()//2) #NumPy always reads first the vertical values from the
WIDTH = round(root.winfo_screenheight()//2) #y axis, and then, the horizontal values from the x axis.
root.destroy()

NUMBER = 1500#((HEIGHT*WIDTH)//100)*random.randint(5, 10)
SPEED = 1#random.uniform(1, 3)
SENSOR_RANGE = 2#random.randint(6, 12)
SENSOR_SIZE = 1
BOUNDARY = False#random.choice((True, False))
START = 'random'#random.choice(('centre', 'random'))
SA = pi/8 #sensor_angle
RA = pi/4 #rotation_angle
print(f'Dimensions {HEIGHT}x{WIDTH}\nNumber: {NUMBER}\nSpeed: {SPEED}\nSensor range: {SENSOR_RANGE}\nSensor size: {SENSOR_SIZE}\nBoundadies: {BOUNDARY}')
# print(pi)
# print(cv.useOptimized())


#@nb.jit(nopython=True)
def sense(x, y, orientation, sensorAngleOffset, width=WIDTH-1, height=HEIGHT-1):
    sensorAngle = orientation + sensorAngleOffset
    sensorDir = (cos(sensorAngle), sin(sensorAngle))
    sensorCentre = (x + sensorDir[0] * SENSOR_RANGE, y + sensorDir[1] * SENSOR_RANGE)
    somme = 0
    offsetX = -SENSOR_SIZE
    offsetY = -SENSOR_SIZE

    for offsetX in range(-SENSOR_SIZE, SENSOR_SIZE+1):
        for offsetY in range(-SENSOR_SIZE, SENSOR_SIZE+1):
            posX = sensorCentre[0] + offsetX
            posY = sensorCentre[1] + offsetY

            if BOUNDARY is True:
                if posX >= width: posX = width
                if posX < 0: posX = 0

                if posY >= height: posY = height
                if posY < 0: posY = 0

                if posX >= 0 and posX < width and posY >= 0 and posY < height:
                    somme += gray.item(round(posX), round(posY))
                    # somme += gray[round(posX), round(posY)]

            else:
                if posX > width: posX -= width
                if posX < 0: posX += width

                if posY > height: posY -= height
                if posY < 0: posY += height

                somme += gray.item(round(posX), round(posY))
                # somme += gray[round(posX), round(posY)]

    return somme


def boundary(x, y, W, H):
    if BOUNDARY is True:
        if x >= W: x = W
        if x < 0: x = 0

        if y >= H: y = H
        if y < 0: y = 0

    else:
        if x > W: x -= W
        if x < 0: x += W

        if y > H: y -= H
        if y < 0: y += H
    return x, y


class Agents:
    def __init__(self): #30° = π/6,  45° = π/4, 60° = π/3, 90° = π/2
        '''Initializes a random orientation, as well as COLOR'''

        self.WIDTH = WIDTH-1
        self.HEIGHT = HEIGHT-1

        if LINES:
            self.x = self.WIDTH//2
            self.y = self.HEIGHT//2

        else:
            if START == 'random':
                self.x = random.randint(0, self.WIDTH)
                self.y = random.randint(0, self.HEIGHT)
            elif START == 'centre':
                self.x = float(self.WIDTH//2)
                self.y = float(self.HEIGHT//2)

        self.orientation = random.random() * 2*pi #random.choice(angle)

    def lines_movement(self):
        '''Movement of lines art.'''

        #Normal distribution with the mean=going straight and the standard deviation being how likely it is to turn
        # self.orientation = random.normalvariate(self.orientation, pi/20) #Bell curve

        # self.orientation += random.choice((pi/8, 0, 0, 0, -pi/8))
        self.x += round(cos(self.orientation)*orient_x)
        self.y += round(sin(self.orientation)*orient_y)

        if self.x >= self.WIDTH:
            self.x = 0
        if self.x < 0:
            self.x = self.WIDTH-1

        if self.y >= self.HEIGHT:
            self.y = 0
        if self.y < 0:
            self.y = self.HEIGHT-1

        if COLOR_BOOL is True:
            canvas[round(self.x), round(self.y)] = [COLOR[0], COLOR[1], COLOR[2]]
        else: canvas[round(self.x), round(self.y)] = COLOR

    def movement(self):
        '''Orientation, interaction with or without other agents, and movement.'''

        if gray.item(round(self.x), round(self.y)) == 255:
            self.orientation = random.random() * 2*pi
        else:
            if COLOR_BOOL is True:
                canvas[round(self.x), round(self.y)] = [COLOR[0], COLOR[1], COLOR[2]]
            else: canvas.itemset(round(self.x), round(self.y), COLOR)

        forward = sense(self.x, self.y, self.orientation, 0.0)
        left = sense(self.x, self.y, self.orientation, SA)
        right = sense(self.x, self.y, self.orientation, -SA)

        if forward > left and forward > right:
            pass

        elif forward < left and forward < right:
            # print('random')
            self.orientation += random.choice((-RA, RA))

        elif left > right:
            # print('left')
            self.orientation += RA

        elif left < right:
            # print('right')
            self.orientation += -RA

        #Normal distribution with the mean=going straight and the standard deviation being how likely it is to turn
        # self.orientation = random.normalvariate(self.orientation, pi/40) #Bell curve

        # self.orientation += random.choice((pi/8, 0, 0, 0, -pi/8))
        # self.x += cos(self.orientation)*SPEED
        # self.y += sin(self.orientation)*SPEED
        # if random.randint(0,100) < 2:
        #     self.orientation += random.choice((self.SA, -self.SA))
        self.x += cos(self.orientation)*SPEED
        self.y += sin(self.orientation)*SPEED
        # self.orientation = tanh(dy - self.y /dx - self.x)

        if BOUNDARY is True:
            if self.x >= self.WIDTH or self.x < 0 or self.y >= self.HEIGHT or self.y < 0:
                self.orientation = random.random() * 2*pi
        self.x, self. y = boundary(self.x, self.y, self.WIDTH, self.HEIGHT)



def movement(agent): #x, y, orientation = agent[0], agent[1], agent[2]
    '''Orientation, interaction with or without other agents, and movement.'''

    if gray.item(round(agent[0]), round(agent[1])) == 255:
        agent[2] = random.random() * 2*pi
    else:
        if COLOR_BOOL is True:
            canvas[round(agent[0]), round(agent[1])] = [COLOR[0], COLOR[1], COLOR[2]]
        else: canvas.itemset(round(agent[0]), round(agent[1]), COLOR)

    forward = sense(agent[0], agent[1], agent[2], 0.0)
    left = sense(agent[0], agent[1], agent[2], SA)
    right = sense(agent[0], agent[1], agent[2], -SA)

    if forward > left and forward > right:
        pass

    elif forward < left and forward < right:
        agent[2] += random.choice((-RA, RA))

    elif left > right:
        agent[2] += RA

    elif left < right:
        agent[2] += -RA

    agent[0] += cos(agent[2])*SPEED
    agent[1] += sin(agent[2])*SPEED

    if BOUNDARY is True:
        if agent[0] >= WIDTH-1 or agent[0] < 0 or agent[1] >= HEIGHT-1 or agent[1] < 0:
            agent[2] = random.random() * 2*pi
    agent[0],  agent[1] = boundary(agent[0], agent[1], WIDTH-1, HEIGHT-1)



COLOR = True
COLOR_BOOL = COLOR
if COLOR:
    COLOR = []
    for _ in range(3):
        i = random.randint(round(255/3), 255)
        COLOR.append(i)
    COLOR = tuple(COLOR)
    CHANNELS = 3
else:
    COLOR = 255
    CHANNELS = 1

LINES = False
if LINES:
    agents = [Agents(COLOR) for _ in range(150)]
    orient_x = random.randint(1, 8)
    orient_y = random.randint(1, 8)
else:
    # agents = [Agents() for _ in range(NUMBER)]
    if START == 'random':
        agents = tuple([{0: random.randint(0, WIDTH-1), 1: random.randint(0, HEIGHT-1), 2:random.random() * 2*pi} for _ in range(NUMBER)])
    elif START == 'centre':
        agents = tuple([{0: WIDTH//2, 1: HEIGHT//2, 2:random.random() * 2*pi} for _ in range(NUMBER)]) #random.uniform(-pi, pi)

#The zeros() function makes a matrix containing only zeros given the matrix’s number of rows and columns. (All bgr values are 0)
canvas = np.zeros((WIDTH, HEIGHT, CHANNELS), dtype='uint8') #3 can be put after width and height to have a COLORed channel, uint8 is a common image type

WINDOW = 'Canvas'
cv.namedWindow(WINDOW, cv.WINDOW_NORMAL)
cv.setWindowProperty(WINDOW, cv.WND_PROP_FULLSCREEN, cv.WINDOW_FULLSCREEN)

# cv.setWindowProperty(WINDOW, cv.WND_PROP_FULLSCREEN, cv.WINDOW_AUTOSIZE)

img_map = cv.imread("Art_map.png", 1)
img_map = cv.resize(img_map, (HEIGHT, WIDTH))
# invert = cv.bitwise_not(img_map)

i = 0
TOTAL = 0
mat = np.ones(canvas.shape, dtype = 'uint8')#*value
if __name__ == '__main__':
    while True:
        start = time.perf_counter()

        cv.imshow(WINDOW, canvas)

        if LINES is False:
            if COLOR_BOOL is True:
                gray = cv.cvtColor(canvas, cv.COLOR_BGR2GRAY)

            #The [:,:] stands for everything. Each : stands for the respective dimensions of the array
            # canvas[:,:] = canvas *.9
            # value = 1

            canvas = cv.subtract(canvas, mat)

            # canvas = cv.GaussianBlur(canvas, (3,3), 0)

        if COLOR_BOOL is False:
            gray = canvas

        try:
            img_map = cv.imread("Art_map.png", 1)
            img_map = cv.resize(img_map, (HEIGHT, WIDTH))
        except:
            pass

        # canvas[:,:] = img_map
        # canvas = cv.add(canvas, img_map)//4
        # gray = cv.subtract(cv.cvtColor(img_map, cv.COLOR_BGR2GRAY), gray)
        # gray[:,:] = gray - 0.9*cv.cvtColor(img_map, cv.COLOR_BGR2GRAY)
        # gray = cv.add(gray, cv.cvtColor(img_map, cv.COLOR_BGR2GRAY))//70 #best one
        # gray = img_map

        # [agent.movement() for agent in agents]
        for agent in agents:
            movement(agent)

        i += 1
        # print(i)
        # time.sleep(0.001)
        # print([pixel for pixel in canvas])
        finish = time.perf_counter()
        print("\r", 1/(finish - start), " fps", end='')
        TOTAL += finish - start
        # if i == 1000: break
        key = cv.pollKey()
        # if sys.version_info >= (3, 10):
        #     match key:
        #         case 27: #escape
        #             break
        #         case 32: #space
        #             while True:
        #                 key = cv.waitKey(1000)
        #                 if key == 32:
        #                     break
        # else:
        if key == 27:
            break
        if key == 32:
            while True:
                key = cv.waitKey(1000)
                if key == 32:
                    break
        # if cv.pollKey() & 0xFF == 27:#ord('q'): #If the key pressed is Q
        #     break                 #27 is the Escape key
    cv.destroyAllWindows() #Destroys all created cv windows
    print(f'\n\nAverage: {TOTAL/i} seconds per frame, or {1/(TOTAL/i)} fps')
    #Sensor range: 11
    #Sensor size: 1