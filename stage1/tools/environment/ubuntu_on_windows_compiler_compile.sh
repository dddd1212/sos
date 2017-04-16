sudo apt-get update
sudo apt-get install make
sudo apt-get install gcc
sudo apt-get install libgmp3-dev
sudo apt-get install libmpfr-dev
sudo apt-get install libisl-dev
sudo apt-get install libcloog-isl-dev
sudo apt-get install libmpc-dev
sudo apt-get install texinfo

cd $HOME
mkdir src
cd src

wget http://gcc.parentingamerica.com/releases/gcc-5.3.0/gcc-5.3.0.tar.bz2
tar xf gcc-5.3.0.tar.bz2

wget http://ftp.gnu.org/gnu/binutils/binutils-2.26.tar.bz2
tar xf binutils-2.26.tar.bz2


mkdir -p $HOME/opt/cross
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

cd $HOME/src
mkdir build-binutils
cd build-binutils
../binutils-2.26/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd $HOME/src
mkdir build-gcc
cd build-gcc
../gcc-5.3.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
