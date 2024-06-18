#!/usr/local/bin/python3
import cv2 as cv
import numpy as np
import random
import sys

from sys import platform
if platform.startswith('win'): #if the program is being run on windows, the tkinter blurriness is removed
    from ctypes import windll
    windll.shcore.SetProcessDpiAwareness(1)
import tkinter
root = tkinter.Tk()
HEIGHT = root.winfo_screenwidth() #NumPy always reads first the vertical values from the
WIDTH = root.winfo_screenheight() #y axis, and then, the horizontal values from the x axis.
root.destroy()
WIDTH = WIDTH
HEIGHT = HEIGHT

del sys.modules['tkinter']
del tkinter

RULES = ""
wrong = False
# print(len(RULES))
try:
    with open("Rules.txt", "r") as rules:
        RULES = rules.readline()
except:
    # print(len(RULES)) #str len 3
    # print(len(RULES))

    if len(RULES) != 0:
        for char in RULES:
            if char != 'R' and char != 'L':
                # print(char, "is not R or L")
                wrong = True
                break
    else:
        # print("Empty?")
        wrong = True

if '\n' in RULES:
    RULES = RULES[:len(RULES)-1]

if wrong == True:
    some_rules = ["RL", "LLRR", "RLLR", "LRRRRRLLR", "LLRRRLRLRLLR", "RRLLLRLLLRRR", "RRRRL"]
    RULES = some_rules[random.randint(0, 6)]

class Ant:
    def __init__(self, direction):
        
        #Starts out in the center
        self.x = (WIDTH-1)//2
        self.y = (HEIGHT-1)//2

        self.direction = direction

        self.color = []
    
    def all(self, colors, rules):

        if self.x > WIDTH-1: self.x -= WIDTH
        if self.x < 0: self.x += WIDTH

        if self.y > HEIGHT-1: self.y -= HEIGHT
        if self.y < 0: self.y += HEIGHT

        self.color = canvas[self.x, self.y]

        index = -1
        for i in range(len(rules)):
            if set(colors[i]) == set(self.color):
                index = i
                break
        
        newIndex = index+1 if index+1 != len(colors) else 0
        if rules[index] == "R": #black
            self.direction += 1
        elif rules[index] == "L":
            self.direction -= 1
        else:
            raise ValueError("The letters must only be R or L!")
        canvas[self.x, self.y] = colors[newIndex]
        
        if self.direction >= 4:
            self.direction = 0
        if self.direction <= -1:
            self.direction = 3
        dir = self.direction

        match dir:
            case 0: #up
                self.x -= 1
            case 1: #right
                self.y += 1
            case 2: #down
                self.x += 1
            case 3: #left
                self.y -= 1

tmp = "Settings"
window = "Canvas"
cv.namedWindow(tmp, cv.WINDOW_NORMAL)
if platform.startswith('win'):
    cv.setWindowProperty(tmp, cv.WND_PROP_FULLSCREEN, cv.WINDOW_FULLSCREEN)

def na(value):
    pass

def main():
    global WIDTH
    global HEIGHT
    global canvas
    
    
    cv.createTrackbar('Width', tmp, 0, HEIGHT, na)
    cv.setTrackbarPos("Width", tmp, HEIGHT)
    cv.createTrackbar('Height', tmp, 0, WIDTH, na)
    cv.setTrackbarPos("Height", tmp, WIDTH)

    cv.createTrackbar("Direction", tmp, 0, 3, na)
    cv.setTrackbarPos("Direction", tmp, 0)

    cv.createTrackbar('Speed', tmp, 0, 5000, na)
    cv.setTrackbarPos("Speed", tmp, 30)
    
    cv.createTrackbar('Start', tmp, 0, 1, na)
    cv.setTrackbarPos("Start", tmp, 0)

    start = 0
    while start == 0:
        start = cv.getTrackbarPos("Start", tmp)
        if cv.pollKey() != -1:
            break

    WIDTH = cv.getTrackbarPos("Height", tmp)
    HEIGHT = cv.getTrackbarPos("Width", tmp)
    speed = cv.getTrackbarPos("Speed", tmp)
    direction = cv.getTrackbarPos("Direction", tmp)

    cv.destroyAllWindows()

    canvas = np.zeros((WIDTH, HEIGHT, 3), dtype='uint8')
    cv.namedWindow(window, cv.WINDOW_NORMAL)
    if platform.startswith('win'):
        cv.setWindowProperty(window, cv.WND_PROP_FULLSCREEN, cv.WINDOW_FULLSCREEN)


    ant = Ant(direction)

    color = []
    colors = [(0, 0, 0)]
    for n in range(len(RULES)-1):
        for _ in range(3):
            i = random.randint(round(255/3), 255)
            color.append(i)
        colors.append(tuple(color))
        color = []
    colors = tuple(colors)
    # print(RULES)

    n = 0
    # cap = 1/60
    while True:
        n += 1

        if n >= speed:
            cv.imshow(window, canvas)
            n = 0
        
        # [ant.pixel() for ant in ants]
        # [ant.movement(colors) for ant in ants]
        ant.all(colors, RULES)

        if cv.pollKey() != -1:
            break
    cv.destroyAllWindows()

if __name__ == '__main__':
    main()

"""
RRRRL
"""