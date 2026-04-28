#!/bin/sh

cc main.c src/glad.c -o glsltester\
  -I./include -I/usr/include -I/usr/X11R6/include -I/usr/X11R7/include\
  -I/usr/local/include\
  -L/usr/lib -L/usr/X11R6/lib -L/usr/X11R7/lib -L/usr/local/lib\
  -lglfw -lm
./glsltester
