# How to Build


## SecOS/SecProcess Loaders + GreenBox Tools


### For Ubuntu 24.04.3
Pre-requisites:
- Install automake-1.15.1
    - Download automake-1.15.1 (https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz), compile it, and sudo make install.
- Install compiler tools
    ```
    sudo apt update
    sudo apt install build-essential libtool crossbuild-essential-i386 gcc-mingw-w64 git pkg-config mtools texinfo patchelf
    sudo apt install libwxgtk3.2-dev libtomcrypt-dev libusb-1.0-0-dev libcurl4-openssl-dev
    sudo apt install libsdl2-dev
    sudo apt install bison flex gawk
    ```

Steps:
1. In the project root folder, copy ```config.mk.example``` to ```config.mk```. Modify config.mk if needed; e.g., setting "export DEBUG = y" to compile the debug version. 
2. Build 3rd party libraries (e.g., ported libraries lke newlib/openssl/libssh2) with ```make first_build```. Run this command only after modifying libraries. 
3. Build the entire project with ```make clean;make```. 

Output files are in the folder "./output/dist/"


### For Ubuntu 22.04.5
Pre-requisites:
- Install automake-1.15.1 and autoconf-2.69
    - Download automake-1.15.1 (https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz), and autoconf-2.69 (https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz). Compile them and install them.
- Replace the default libtool with libtool-2.4.7:
    - Download from https://mirror.team-cymru.com/gnu/libtool/libtool-2.4.7.tar.gz. 
    ```
    sudo apt remove libtool
    
    cd libtool-2.4.7
    ./configure --prefix=/usr
    make
    sudo make install
    ```
- Install compiler tools
    ```
    sudo apt remove automake autoconf
    sudo apt install build-essential libtool crossbuild-essential-i386 gcc-mingw-w64 gcc-12 git pkg-config mtools texinfo
    sudo apt install libwxgtk3.0-gtk3-dev libtomcrypt-dev libusb-1.0-0-dev patchelf
    sudo apt install libsdl2-dev
    sudo apt install bison flex gawk
    ```


# How to Run
1. Boot GreenBox SecOS.
2. After GreenBox DesktopMgr shows, run SecApps in "./output/dist/sap/"; e.g., ```sudo ./sl_demo_sp```.
    - Or run run_demo_*.sh in the project root folder as a quick way; e.g., ```sudo ./run_demo_sp_demo.sh```.