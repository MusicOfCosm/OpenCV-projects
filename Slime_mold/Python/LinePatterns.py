import cv2 as cv
import numpy as np
import random
import math
import time

from sys import platform
if platform.startswith('win'): #if the program is being run on windows, the tkinter blurriness is removed
    from ctypes import windll
    windll.shcore.SetProcessDpiAwareness(1)
import tkinter
root = tkinter.Tk()
HEIGHT = root.winfo_screenwidth() #NumPy always reads first the vertical values from the
WIDTH = root.winfo_screenheight() #y axis, and then, the horizontal values from the x axis.
root.destroy()


# HEIGHT = 1920
# WIDTH = 1080

color = []
for _ in range(3):
    i = random.randint(round(255/3), 255)
    color.append(i)
color = tuple(color)

class Line:
    def __init__(self, angle):
        
        #Starts out in the center
        self.x = (WIDTH-1)//2
        self.y = (HEIGHT-1)//2

        self.orientation = angle#random.uniform(-math.pi, math.pi)

    def lines_movement(self, orient_x, orient_y):

        self.x += round(math.cos(self.orientation)*orient_x) #round was the good mistake
        self.y += round(math.sin(self.orientation)*orient_y)
        
        if self.x > WIDTH-1: self.x -= WIDTH-1
        if self.x < 0: self.x += WIDTH-1

        if self.y > HEIGHT-1: self.y -= HEIGHT-1
        if self.y < 0: self.y += HEIGHT-1

        canvas[round(self.x), round(self.y)] = [color[0], color[1], color[2]]

agents = []
for i in range(1, 51):
    agents.append(Line(((2*math.pi)/50)*i))
orient_x = random.uniform(1, 6)
orient_y = random.uniform(1, 6)

canvas = np.zeros((WIDTH, HEIGHT, 3), dtype='uint8')

window = 'Canvas'
cv.namedWindow(window, cv.WINDOW_NORMAL)
cv.setWindowProperty(window, cv.WND_PROP_FULLSCREEN, cv.WINDOW_FULLSCREEN)


def main():
    cap = 1/300
    while True:
        start = time.perf_counter()

        cv.imshow(window, canvas)
        
        [agent.lines_movement(orient_x, orient_y) for agent in agents]

        finish = time.perf_counter()
        iter_time = finish - start

        if iter_time < cap:
            diff = cap - iter_time
            time.sleep(diff)

        if cv.pollKey() != -1:
            break
    cv.destroyAllWindows()

if __name__ == '__main__':
    main()