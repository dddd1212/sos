file /cygdrive/d/dror/projects/SOS/stage1/Qube/_output/bootloader/boot.elf
set step-mode on
set disassembly-flavor intel
break hack_for_gdb
commands
	silent
	eval "add-symbol-file /cygdrive/d/dror/projects/SOS/stage1/Qube/_output/system/%s 0x%lx", module_file_name, entry
	c
end
