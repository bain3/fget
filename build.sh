#!/bin/sh

# get distro information
. /etc/os-release
FGET_VERSION="1.0-0"

# install build dependencies
if [ "$ID" = "debian" ] || [ "$ID_LIKE" = "debian" ]; then
  apt update
  apt install cmake g++ wget libssl-dev zip git libc6 -y
elif [ "$ID" = "arch" ] || [ "$ID_LIKE" = "arch" ]; then
  pacman -Sy cmake gcc wget openssl unzip glibc make --noconfirm
fi

# Fetch cryptopp and cmake files
mkdir cryptopp && cd cryptopp || exit
wget https://cryptopp.com/cryptopp840.zip
unzip cryptopp840.zip
wget https://github.com/noloader/cryptopp-cmake/archive/CRYPTOPP_8_4_0.zip
unzip -j CRYPTOPP_8_4_0.zip

# Build cryptopp
mkdir cmake-build && cd cmake-build || exit
cmake ..
make cryptopp-static
make install

# Build fget
cd ../..
cmake .
make

# Ubuntu / Debian systems
# Build .deb
if [ "$ID" = "ubuntu" ] || [ "$ID" = "debian" ]; then
  mkdir -p fget_$FGET_VERSION/usr/local/bin
  mv fget fget_$FGET_VERSION/usr/local/bin
  mkdir fget_$FGET_VERSION/DEBIAN
  echo "
Package: fget
Version: $FGET_VERSION
Architecture: all
Section: base
Priority: optional
Depends: libc6 (>= $(dpkg-query --showformat='${Version}' --show libc6)), libssl1.1 (>= 1.1)
Maintainer: bain <bain@bain.cz>
Description: fget is a utility for downloading from f.bain-like websites"\
  > fget_$FGET_VERSION/DEBIAN/control
  dpkg --build fget_$FGET_VERSION
fi
