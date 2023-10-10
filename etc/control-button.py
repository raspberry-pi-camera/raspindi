#!/usr/bin/env python3

from gpiozero import Button
from subprocess import check_call
from signal import pause

is_NDI = True

def toggle_NDI():
    global is_NDI
    if is_NDI:
        #Turn off NDI and turn on HDMI
        check_call(['sudo', 'systemctl', 'stop', 'RasPi-NDI.start.service'])
        check_call(['sudo', 'systemctl', 'start', 'RasPi-HDMI.start.service'])
        print("HDMI mode")
    else:
        #Turn on NDI and turn off HDMI
        check_call(['sudo', 'systemctl', 'stop', 'RasPi-HDMI.start.service'])
        check_call(['sudo', 'systemctl', 'start', 'RasPi-NDI.start.service'])
        print("NDI mode")
    is_NDI = not is_NDI

def shutdown_pi():
    #Shutdown the system
    print("Shutdown")
    check_call(['sudo', 'poweroff'])

button = Button(21)
button.hold_time = 5
check_call(['sudo', 'systemctl', 'start', 'RasPi-NDI.start.service'])

try:
    button.when_released = toggle_NDI
    button.when_held = shutdown_pi
    pause()
finally:
    pass
    