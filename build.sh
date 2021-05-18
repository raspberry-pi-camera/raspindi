#!/usr/bin/env sh

if [ ! -d "build" ]; then
  mkdir build
fi

g++ -std=c++11 -pthread  -Wl,--allow-shlib-undefined -Wl,--as-needed -Iinclude/ -Llib/ndi -L /opt/vc/lib -o build/raspindi src/main.cpp -lndi -ldl -lconfig++ -lmmal_core -lmmal -lmmal_util
