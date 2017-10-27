
PKG_NAME="volumectrl"
PKG_VERSION="1.0"
PKG_ARCH="any"
PKG_LICENSE="BSD"
PKG_DEPENDS_TARGET="toolchain systemd alsa-lib wiringPi"
PKG_SECTION="tools"
PKG_SHORTDESC="Controls the volume via two GPIOs on the raspberry pi"
PKG_LONGDESC="Controls the volume via two GPIOs on the raspberry pi"

PKG_IS_ADDON="no"
PKG_AUTORECONF="no"



make_target() {
	$CC $PKG_DIR/src/main.c -lpthread -lasound -lwiringPi $CFLAGS -o volumectrl
}

makeinstall_target() {
	mkdir -p $INSTALL/usr/bin
	cp -P volumectrl $INSTALL/usr/bin
}

post_install() {
	enable_service volumectrl.service
}
