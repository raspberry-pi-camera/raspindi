#!/usr/bin/env sh

INSTALL_DIR="/opt/raspindi"
BIN_DIR="$INSTALL_DIR/bin"
LIB_DIR="$INSTALL_DIR/lib"

mkdir "$INSTALL_DIR"
mkdir "$LIB_DIR"
mkdir "$BIN_DIR"

if [ $(uname -m) == "aarch64" ]; then
  cp lib64/ndi/* "$LIB_DIR"
else
  cp lib32/ndi/* "$LIB_DIR"
fi

cp build/src/raspindi "$BIN_DIR"
cp build/src/libndioutput.so "$LIB_DIR"

cp etc/raspindi.conf.default /etc/raspindi.conf

echo '#!/usr/bin/env sh
LD_LIBRARY_PATH="/opt/raspindi/lib" /opt/raspindi/bin/raspindi
' > "$INSTALL_DIR/raspindi.sh"
chmod +x "$INSTALL_DIR/raspindi.sh"
