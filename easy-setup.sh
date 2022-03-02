#!/usr/bin/env sh

set -eu

sudo apt update
sudo apt install -y libconfig++-dev libboost-program-options-dev libavahi-client3 cmake libcamera-dev libevent-pthreads-2.1-7 libevent-core-2.1-7 libevent-dev

./build.sh
sudo ./install.sh

echo "Done"
