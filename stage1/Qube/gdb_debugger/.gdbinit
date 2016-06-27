b 0x7c00
file /Qube/_output/bootloader/boot.elf
# Dont cry. in cygwin shell do this: ln -s /cygdrive/d/dror/projects/SOS/stage1/Qube /Qube
set step-mode on
set disassembly-flavor intel
break hack_for_gdb
commands
	silent
	eval "add-symbol-file /Qube/_output/system/%s 0x%lx", module_file_name, entry
	c
end
