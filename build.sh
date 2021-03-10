#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

g++ -std=c++11 -pthread  -Wl,--allow-shlib-undefined -Wl,--as-needed -Iinclude/ -Llib/ndi -Llib/opencv -Llib/raspicam -o build/raspindi src/main.cpp -lndi -ldl -lconfig++ -lraspicam