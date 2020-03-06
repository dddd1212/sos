start working:
install python27
install VS15
 * install also the Visual C++ tools for Android for the gdb debugger
 * run VC_IoT.vsix from tools\gdb_for_vc

===================================Compiling===================================

compile all:
	Add python to the enviroment variable "Path".
	After this make.py will insert all the modules to the virtual harddisk and install the boot.


To compile using Cygwin:
	Install Cygwin and compile cross compiler according to http://wiki.osdev.org/GCC_Cross-Compiler
	Add Cygwin64\bin, C:\cygwin64\home\=YOUR_USER=\opt\cross\bin to the enviroment variable "Path"
	Install gdb package at Cygwin

To compile with ubuntu on windows:
	install ubuntu on windows
	run tools\environment\UbuntuOnWindowsSetup.bat (this will download and compile gcc and it's dependencies)

===================================Debugging===================================

debugging with bochs:
	you need to compile bochs with gdb stub.
	create enviroment variable named %BOCHSGDB% that points to the bochs.exe with the gdb stub.

debugging with vmware:
	install vmware