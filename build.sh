#!/usr/bin/env sh

if [ ! -d "build" ]; then
  mkdir build
fi

if [ $(uname -m) == "aarch64" ]; then
  ln -s lib64 lib
else
  ln -s lib32 lib
fi

cd build
cmake ..
make