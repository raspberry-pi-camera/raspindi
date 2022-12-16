#!/usr/bin/env sh

if [ ! -d "build" ]; then
  mkdir build
fi

if [ -d "lib-$(uname -m)" ]; then
  ln -s lib-$(uname -m) lib
else
  ln -s lib-armv7 lib
fi

cd build
cmake ..
make