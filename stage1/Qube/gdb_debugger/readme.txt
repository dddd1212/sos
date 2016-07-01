start working:
install python27
install VS15
 * install also the Visual C++ tools for Android for the gdb debugger
 * run VC_IoT.vsix from tools\gdb_for_vc
install Cygwin and compile cross compiler according to http://wiki.osdev.org/GCC_Cross-Compiler
add python, Cygwin64\bin, C:\cygwin64\home\=YOUR_USER=\opt\cross\bin to the enviroment variable "Path"
install gdb package at Cygwin

compile all:
create empty hd with the createhd.py script at tools directory and put it at "../_output/_disk/disk.vhd"
after this make.py will build all the modules and install the boot.

debugging with bochs:
you need to compile bochs with gdb stub.
create enviroment variable named %BOCHSGDB% that points to the bochs.exe with the gdb stub.

debugging with vmware:
1. Install VMWare-VIX
2. Create new VM and the first line in qube_gdb_vmware.bat to it's location.
3. change the HD of the VM to this:
"""
# Disk DescriptorFile
version=1
encoding="UTF-8"
CID=c9b58254
parentCID=ffffffff
isNativeSnapshot="no"
createType="vmfs"

# Extent description
RW 1048576 VMFS =====PATH_TO_DISK_VHD=====

# The Disk Data Base 
#DDB

ddb.adapterType = "lsilogic"
ddb.geometry.cylinders = "512"
ddb.geometry.heads = "64"
ddb.geometry.sectors = "32"
ddb.longContentID = "e2f65ac84feef46a1815c03cc9b58254"
ddb.uuid = "60 00 C2 9a 81 87 b4 5a-a0 59 e6 9d da b6 d0 47"
ddb.virtualHWVersion = "11"
"""
and set =====PATH_TO_DISK_VHD===== to the correct value. 