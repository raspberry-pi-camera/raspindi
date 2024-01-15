#!/usr/bin/env sh

INSTALL_DIR="/opt/RasPi-NDI-HDMI"
BIN_DIR="$INSTALL_DIR/bin"
LIB_DIR="$INSTALL_DIR/lib"

mkdir "$INSTALL_DIR"
mkdir "$LIB_DIR"
mkdir "$BIN_DIR"

if [ -d "lib-$(uname -m)" ]; then
  cp lib-$(uname -m)/ndi/* "$LIB_DIR"
else
  cp lib-armv7/ndi/* "$LIB_DIR"
fi

cp build/src/raspindi "$BIN_DIR"
cp build/src/libndioutput.so "$LIB_DIR"

cp etc/raspindi.conf.default /etc/raspindi.conf
cp etc/*.service /etc/systemd/system/
cp etc/control-button.py "$INSTALL_DIR"
chmod +x "$INSTALL_DIR/control-button.py"
systemctl enable RasPi-NDI-HDMI-button.start.service

echo '#!/usr/bin/env sh
LD_LIBRARY_PATH="/opt/RasPi-NDI-HDMI/lib" /opt/RasPi-NDI-HDMI/bin/raspindi
' > "$INSTALL_DIR/RasPi-NDI-HDMI.sh"
chmod +x "$INSTALL_DIR/RasPi-NDI-HDMI.sh"