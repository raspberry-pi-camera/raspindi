#!/usr/bin/env sh

INSTALL_DIR="/opt/RasPi-NDI-HDMI"

rm -rf "$INSTALL_DIR"
rm -f /etc/raspindi.conf
systemctl disable RasPi-NDI-HDMI-button.start.service
rm -f /etc/systemd/system/RasPi-NDI-HDMI-button.start.service
rm -f /etc/systemd/system/RasPi-NDI.start.service
rm -f /etc/systemd/system/RasPi-HDMI.start.service