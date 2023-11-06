# Recommended Parts list
These are the parts I used to build the camera. You could substitute other functionally equivalent parts if needed (USB power instead of POE+ HAT and header extensions, other microSD cards, lenses, camera modules, etc).
- Raspberry Pi 4B [Adafruit](https://www.adafruit.com/product/4296)
- Raspberry Pi High Quality HQ Camera [Adafruit](https://www.adafruit.com/product/4561)
- Raspberry Pi PoE+ HAT [Adafruit](https://www.adafruit.com/product/5058)
- Raspberry Pi 4 Pro Mounting Plate for HQ Camera [Adafruit](https://www.adafruit.com/product/5026)
- GPIO Female Socket Riser Header - 2x2 4-pin [Adafruit](https://www.adafruit.com/product/4855)
- GPIO Stacking Header for Pi A+/B+/Pi 2/Pi 3 - Extra-long 2x20 Pins [Adafruit](https://www.adafruit.com/product/2223)
- Waveshare Industrial Zoom Lens 8-50mm [Amazon](https://a.co/d/9OnPh5z)
- PNY 32 GB microSD Card [Amazon](https://a.co/d/dV6UQZt)
- M2.5 Brass Standoff kit [Amazon](https://a.co/d/1kkJjir)
- Momentary switches with 2 pin female header [Amazon](https://a.co/d/fanJkFU)

# Build instructions
1. Use brass standoffs to attach the camera module to the mounting plate.
2. Attach the FPC cable (included with camera module) to the camera module. The blue panel shoul face away from the camera module.
3. Attach the other end of the FPC cable to the Rasberry Pi's camera connector (near the USB ports). The blue panel should face the USB ports.
4. Use brass standoffs to attach the Raspberry Pi to the mounting plate. Take care to route the FPC cable towards the microSD card slot, between the Raspberry Pi and the mounting plate.
   - If you are using the POE+ HAT, make sure to use the appropriate standoffs and header extensions so they can stack properly.
5. Attach a switch to the two GPIO pins closest to the ethernet port.
6. Insert the prepared microSD card into the microSD card slot.
7. Attach the lens to the camera module.
   - The Waveshare lens above is a C-mount lens and uses the ring included with the HQ camera module. Other lenses may vary.
9. Build is complete! Power on the camera using the chosen power method.
    - If using the camera in HDMI mode, connect a micro HDMI to HDMI adapter and press the switch to change modes once it it fully powered on.
