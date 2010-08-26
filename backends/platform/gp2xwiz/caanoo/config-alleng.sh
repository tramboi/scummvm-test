#!/bin/sh

echo Quick script to make running configure all the time less painful
echo and let all the build work be done from the backend/build folder.

# Assume Caanoo toolchain/build env.
. /opt/arm-caanoo/environment-setup

# Export the tool names for cross-compiling
export DEFINES=-DNDEBUG

# Edit the configure line to suit.
cd ../../../..
./configure --backend=caanoo --disable-mt32emu --host=caanoo --disable-alsa --disable-flac --disable-nasm --disable-vorbis --disable-hq-scalers --with-sdl-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr/bin --with-mpeg2-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr --enable-tremor --with-tremor-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr --enable-zlib --with-zlib-prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr --enable-mad --with-mad-prefix=/opt/arm-caanoo/arm-none-linux-gnueabi/usr --enable-all-engines --enable-vkeybd --enable-plugins --default-dynamic

echo Generating config for GP2X Caanoo complete. Check for errors.
