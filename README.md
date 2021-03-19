# RaspiNDI

RaspiNDI is a simple NDI send library for Raspberry Pi. It was initially
developed to allow a streaming setup for a church during the COVID-19 outbreak
in 2020. 

It uses the NDI library, allowing for auto-discovery of streams on the local
network.

## Getting started

These intructions are for a clean installation of
[Raspberry Pi OS](https://www.raspberrypi.org/software/).
All steps are performed on the command line.

### tl;dr

- Clone this repo.
- `./easy-setup.sh`
- `/opt/raspindi/raspindi.sh`

### Step by step

Make sure git is installed.

```
sudo apt update
sudo apt install git
```

Clone this repository and `cd` into it.

```
git clone https://github.com/raspberry-pi-camera/raspindi.git
cd raspindi
```

Install compilation dependencies.

```
sudo apt update
sudo apt install libconfig++-dev
```

Compile.

```
./build.sh
```

Install.

```
sudo ./install.sh
```

Install runtime dependencies.

```
sudo apt update
sudo apt install libjasper-runtime
```

Run it. (It does not require root to run.)

```
/opt/raspindi/raspindi.sh
```

Open an NDI receiver somewhere on the same network. It should detect the 
Raspberry Pi camera after a few seconds.

[OBS Studio](https://obsproject.com/) with the 
[OBS-NDI plugin](https://github.com/Palakis/obs-ndi/releases/) 
works well.
