# How to Build

## For Windows
Pre-requisites:
Pre-requisites:
- MSYS2 (msys2-x86_64-20250221.exe or above)
    - Whenever a newer version of msys2 is used, one must replace the libraries in ```code/win/cygwin``` and ```code/win/libusb``` with the corresponding MSYS2 libraries.
- Visual Studio 2022 (Community version or above) + Win10 WDK
    - See [Step 1 to Step 3](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk#download-icon-for-visual-studio-step-1-install-visual-studio-2022) about how to install Win10 SDK and WDK.
- Qt 6 and above with Qt Creator 16 and above
    - Install via the online installer on https://www.qt.io/download-qt-installer-oss
    - Use the MSVC2022 Qt kit "Desktop Qt XXXXXX MSVC2022 64bit", not the "Desktop Qt XXXXXX MinGW 64-bit".
    - [Recommend] Install debug symbols of Qt.

Steps:
1. After building GreenBox-SDK for Linux, copy Linux build files to the GreenBox folder for the Windows build. Copy the folders ```output/dist/sap``` and ```output/dist/sos``` of the Linux build output to the ```win/imgs/sap``` and ```win/imgs/sos``` folders.
2. Copy ```config.mk.example``` to ```config.mk```, check ```config.mk``` to ensure that only "CURRENT_PLATFORM", "TARGET_PLATFORM", and "OS" are updated for the windows build. Other configurations should be the same with the ones for the Linux build. Also copy ```os_module/include/sa_configs.h.example``` to ```os_module/include/sa_configs.h``` and edit the .h file to enable configs for SecAgent.
3. Run Visual Studio 2022 to open and build projects for Windows. Build the x64 release version for all following projects:
    - ```loaders/sos_loader_gui_client/win/SOSLoader_GUI/SOSLoader_GUI.sln```
    - ```os_module/win/SecAgent.sln```
        - Need to copy ```win_config_xmhf_log.h.example``` to ```win_config_xmhf_log.h``` in the ```os_module/include/os_funcs/os/``` folder.
    - ```code/tools_dist/tpm/sub_tools/efi_tpm2_event_logs/efi_tpm2_event_logs.sln```
4. Run Qt Creator to build ```secapp_demos/rdesktop/app/ui/RDesktop/CMakeLists.txt``` and ```secapp_demos/rdesktop/app/ui/RFiles/CMakeLists.txt```. Build the x64 release version for all these projects.
    - Configure projects to Enable "Debug" and "Release", and set output folder to be "Debug" and "Release", not "Desktop_Qt_X_Y_Z_MSVC2022_64bit-*".
5. Open a MSYS2-ucrt64 terminal, go to the ```code/``` folder, run the build command ```make win``` to build loaders for Windows.
6. Open a "Qt 6.10.0 (MSVC 2022 64-bit)" terminal, go to the root folder and run ```post_build_win_qt.bat``` command.

## For Ubuntu 24.04.3
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
1. In the project root folder, copy ```config.mk.example``` to ```config.mk```. Modify config.mk if needed; e.g., setting "export DEBUG = y" to compile the debug version. Also copy ```os_module/include/sa_configs.h.example``` to ```os_module/include/sa_configs.h``` and edit the .h file to enable configs for SecAgent.
2. Build 3rd party libraries (e.g., ported libraries lke newlib/openssl/libssh2) with ```make first_build```. Run this command only after modifying libraries. 
3. Build the entire project with ```make clean;make```. 

Output files are in the folder "./output/dist/"


## For Ubuntu 22.04.5
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
 
 
# Supported Machines

| Machines                                                      | Windows 11 (x64)  | Windows 10 (x64)  | Ubuntu 24.04.3 (x64) |
| --------                                                      | ------            | -------           | ------- |
| Dell Optiplex 7050 <br> (i7-7700, 8GB RAM)                    |  -                |  Y                |   Y   |
| Puget Systems <br> (i9-9900K, Gigabyte Z390, 128GB RAM)       |  -                |  -                |   Y   |
| HP EliteBook 840 Aero G8 <br> (i7-1185G7, 64GB RAM)           |  -                |  Y (with mHV v2)  |   -   |

"-" means not tested.

We test supported machines by running RICH_GUI_SAP only.