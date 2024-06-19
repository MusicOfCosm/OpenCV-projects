# Slime mold

In April 2022, I got interested in [Sebastian Lague](https://www.youtube.com/@SebastianLague)'s [Slime mold simulation](https://www.youtube.com/watch?v=X-iSQQgOd1A), I wanted to replicate it. After searching and searching, I decided to use [OpenCV](https://opencv.org/), since sentdex had a [tutorial](https://pythonprogramming.net/loading-images-python-opencv-tutorial/) on it.

I managed to make it work, but it was extremely slow. Seeing C++ in the documentation, I tried to use it; I was stubborn and didn't want to follow all the tutorials which told me to use [Visual Studio](https://visualstudio.microsoft.com/), but I couldn't really figure out how to make it work with [CMake](https://cmake.org/) and [Sublime Text](https://www.sublimetext.com/). Eventually, I learned [Cython](https://cython.org/), and made the script run way faster, but I still wasn't satisfied.

Eventually, I bit the bullet and learned C/C++ using Visual Studio, I managed to make it work and I was happy, but I had started this project as a gift for my sister, who happens to have a Mac. I learned the hard way that computers can't run programs from different operating systems and/or architectures.


## Python and Cython

I started with `Generative_art.py`, this time I tried to use classes, unlike the absolute mess that I did with the GUI Q&A project, and I think it went alright.

Before implementing the `sense()` method, I wanted to test `movement()` to see if the agents would appear on screen, spawning them in the middle of the screen going in a random direction. Lo and behold, by complete accident, I made them draw out interesting shapes! I decided to keep this so I made another file for it named `LinePatterns.py`.

Coming back to the first file, I managed to make it work, but it is abysmally slow, even with only about a thousand agents.

After first failing to use C/C++, I searched how to use C in Python, and that's how I found Cython. I took me a while to understand how things worked, but I think I managed well enough (though I didn't understand the error messages). Still, despite being almost twenty times faster than Python, it wasn't fast enough. I was especially disappointed that I had to copy the arrays every time. opencv-python couldn't use its function with `memoryview()` objects, and trying to declare the arrays at a global scope didn't work.

`Generative_art_cy.pyx` is the Cython version of `Generative_art.py`, `setup.py` is used to compile it, and `Actual_art.py` to run it.

## C++

This was my very first time using [Visual Studio](https://visualstudio.microsoft.com/), it took me time to learn, and I honestly don't remember what tutorials I used, but I eventually did it! It was way faster than Python or Cython.
I played around with the rudimentary GUI that you can make with OpenCV to change values at runtime, which you can bring up by pressing **V**, you can exit by pressing the **escape key**.

I added other "species", which is just other colors. You can change individual species colors and choose if they should avoid each other or not.


## Using an image

The [paper](https://uwe-repository.worktribe.com/output/980579) that first presented a computer model to represent the slime mold also made use of images (or "Pre-patterning Stimuli") to affect the agents' behavior. I tested it out using Python and Cython, but didn't do much with it: It was much more fun playing with the C++ version and its three species, and I had to study for exams.

You can modify `Art_map.png` to affect the agents' behavior, you can (un)comment the code at line 291 for `Generative_art.py` and 256 for `Generative_art_cy.pyx`.