#!/bin/bash
#script for creating a build-environment for minos, from scratch.
#NOTE: even if this script fails partway, it can serve as a guide of what needs to be installed.

#-- SETTINGS --

# where to install the resulting tools
INSTALL_LOCATION="$HOME/Documents/minos-tools"

#target triplet for binutils/gcc to build
TARGET=x86_64-elf

#download mirror and filenames for binutils/gcc
#TODO: find a way to automatically get the latest
SOURCE_BINUTILS=https://ftp.gnu.org/gnu/binutils/
FILE_BINUTILS=binutils-2.37
SOURCE_GCC=https://ftp.gnu.org/gnu/gcc/gcc-11.2.0/
FILE_GCC=gcc-11.2.0
FILE_EXTENSIONS=.tar.xz

#-- END SETTINGS --

#first up: lets create a working directory, for easy cleanup later
echo "Welcome to minos environment setup, to tweak env settings, please edit this file."
echo "The setup process will now begin, please note this can take some time depending on your system."

export PATH="$INSTALL_LOCATION/bin:$PATH"
export PREFIX=$INSTALL_LOCATION
export TARGET=$TARGET

mkdir -p $INSTALL_LOCATION
cd $INSTALL_LOCATION
mkdir build-gcc
mkdir build-binutils

#download the required files
echo "Downloading source files ..."
wget $SOURCE_BINUTILS$FILE_BINUTILS$FILE_EXTENSIONS
wget $SOURCE_GCC$FILE_GCC$FILE_EXTENSIONS

#... and extract them (and cleanup downloaded files)
echo "Extracting ..."
tar -xvf $FILE_BINUTILS$FILE_EXTENSIONS
tar -xvf $FILE_GCC$FILE_EXTENSIONS
#rm -rf $FILE_BINUTILS$FILE_EXTENSIONS
#rm -rf $FILE_GCC$FILE_EXTENSIONS

#now we install some dependencies
echo "Installing dependencies ..."
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo xorriso mtools qemu qemu-system

#setup limine
echo "Cloning limine ..."
git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
cd limine
make install

#setup gnu-efi
echo "Cloning gnu-efi ..."
cd $INSTALL_LOCATION
git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
cd gnu-efi
make

#setup binutils
echo "Building binutils (please note this can take a long time) ..."
cd $INSTALL_LOCATION/build-binutils
../$FILE_BINUTILS/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j $(nproc)
make -j $(nproc) install
cd ..
rm -rf build-binutils

#setup gcc
echo "Building GCC (this can also take a long time) ..."
cd $INSTALL_LOCATION/build-gcc
../$FILE_GCC/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j $(nproc) all-gcc
make -j $(nproc) all-target-libgcc
make -j $(nproc) install-gcc
make -j $(nproc) install-target-libgcc
cd ..
rm -rf build-gcc

#finish
echo "Environment setup complete, point root makefile to the selected install directory!"
