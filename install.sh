#!/usr/bin/env sh

INSTALL_DIR="/opt/raspindi"
BIN_DIR="$INSTALL_DIR/bin"
LIB_DIR="$INSTALL_DIR/lib"

mkdir "$INSTALL_DIR"
mkdir "$LIB_DIR"
mkdir "$BIN_DIR"

cp lib/ndi/* "$LIB_DIR"

cp build/raspindi "$BIN_DIR"

cp etc/raspindi.conf.default /etc/raspindi.conf

echo '#!/usr/bin/env sh
LD_LIBRARY_PATH="/opt/raspindi/lib" /opt/raspindi/bin/raspindi
' > "$INSTALL_DIR/raspindi.sh"
chmod +x "$INSTALL_DIR/raspindi.sh"
