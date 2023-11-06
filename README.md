# RaspiNDI

RasPi-NDI-HDMI is a fork from the [raspindi](https://github.com/raspberry-pi-camera/raspindi). It includes auto start services for NDI and HDMI output, controlled by a momentary push button on GPIO pin 21. 

It uses the NDI library, allowing for auto-discovery of streams on the local network.

**❗ This software does not work with USB cameras. It uses the native libary for the on-board camera port. ❗**

## Supported boards

This software is tested with Raspberry Pi 3B+ boards. Some users have had success running it on a Rapsberry Pi 4. **The Pi Zero W does not have enough memory to run this software**

## Latency Notes
Raspberry Pi 3b+ = ~600ms   
Raspberry Pi 4b (4GB Model Tested) = ~200ms

## Getting started - compile your own

These intructions are for a clean installation of [Raspberry Pi OS](https://www.raspberrypi.org/software/). All steps are performed on the command line.

### tl;dr

- `git clone https://github.com/SBCV-apegram/RasPi-NDI-HDMI.git && cd RasPi-NDI-HDMI`
- `./easy-setup.sh`
- `/opt/RasPi-NDI-HDMI/RasPi-NDI-HDMI.sh`

### Step by step

Make sure git is installed.

```
sudo apt update
sudo apt install git
```

Clone this repository and `cd` into it.

```
git clone https://github.com/SBCV-apegram/RasPi-NDI-HDMI.git && cd RasPi-NDI-HDMI
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
/opt/RasPi-NDI-HDMI/RasPi-NDI-HDMI.sh
```

Open an NDI receiver somewhere on the same network. It should detect the Raspberry Pi camera after a few seconds.

[OBS Studio](https://obsproject.com/) with the [OBS-NDI plugin](https://github.com/Palakis/obs-ndi/releases/) works well.


# Changelog

## v1.1.0 Rebase from V3.0.3
- Re-added customizations
