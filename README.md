# RaspiNDI

RaspiNDI is a simple NDI send library for Raspberry Pi. It was initially
developed to allow a streaming setup for a church during the COVID-19 outbreak
in 2020. 

It uses the NDI library, allowing for auto-discovery of streams on the local
network.

## Supported boards

This software is tested with Raspberry Pi 3B+ boards. Some users have had success
running it on a Rapsberry Pi 4. **The Pi Zero W does not have enough memory to run
this software**

## Getting started

These intructions are for a clean installation of
[Raspberry Pi OS](https://www.raspberrypi.org/software/).
All steps are performed on the command line.

### tl;dr

- `git clone https://github.com/raspberry-pi-camera/raspindi.git && cd raspindi`
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
git clone https://github.com/raspberry-pi-camera/raspindi.git && cd raspindi
```

Install compilation dependencies.

```
sudo apt update
sudo apt install libconfig++-dev cmake libboost-program-options-dev libevent-dev libcamera-dev
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
sudo apt install libavahi-client3 libevent-pthreads-2.1-7 libevent-core-2.1-7
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


# Changelog

## v2.0.1
Added config parameters for various camera settings (AWB, exposure etc.)

## v2.0.0
Completely new method of acquiring the images - now calling `mmal` directly.  
This, as well as using YUV colour, drastically improves the speed of the system,
and the smoothness of the outputted video.

## v1.1.1
Upgraded NDI library to v4.6.2

## v1.1.0
Removed OpenCV integration
