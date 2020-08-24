#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

g++ -std=c++11 -pthread  -Wl,--allow-shlib-undefined -Wl,--as-needed -Iinclude/ -Llib/ndi -Llib/opencv -Llib/raspicam -o build/raspindi src/main.cpp -lndi -ldl -lraspicam_cv -lopencv_core -lopencv_imgproc -lconfig++