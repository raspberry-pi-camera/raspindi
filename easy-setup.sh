#!/usr/bin/env sh

set -eu

sudo apt update
sudo apt install libconfig++-dev libjasper-runtime
./build.sh
sudo ./install.sh

echo "Done"
