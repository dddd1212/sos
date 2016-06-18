file /cygdrive/d/dror/projects/SOS/stage1/Qube/_output/bootloader/boot.elf
add-symbol-file /cygdrive/d/dror/projects/SOS/stage1/Qube/_output/system/libc.qkr 0xfffff000001043d0
add-symbol-file /cygdrive/d/dror/projects/SOS/stage1/Qube/_output/system/screen.qkr 0xfffff00000305470 -readnow
add-symbol-file /cygdrive/d/dror/projects/SOS/stage1/Qube/_output/system/kernel.qkr 0xfffff000005062f0

file /cygdrive/e/gilad/sos-git2/SOS/stage1/Qube/_output/bootloader/boot.elf
#add-symbol-file /cygdrive/e/gilad/sos-git2/SOS/stage1/Qube/_output/system/libc.qkr 0xfffff000001053d0
#add-symbol-file /cygdrive/e/gilad/sos-git2/SOS/stage1/Qube/_output/system/screen.qkr 0xfffff000003064c0
#add-symbol-file /cygdrive/e/gilad/sos-git2/SOS/stage1/Qube/_output/system/kernel.qkr 0xfffff000005072f0
#add-symbol-file /cygdrive/e/gilad/sos-git2/SOS/stage1/Qube/_output/system/intrpts.qkr 0xfffff00000709ae0
set step-mode on
set disassembly-flavor intel
set disassembly-flavor intel
break hack_for_gdb
commands
	silent
	eval "add-symbol-file /cygdrive/e/gilad/sos-git2/SOS/stage1/Qube/_output/system/%s 0x%lx", module_file_name, entry
	c
end
#break hack_for_gdb
#-break-insert hack_for_gdb
#-interpreter-exec console "break hack_for_gdb"
#-interpreter-exec console commands
#-interpreter-exec console silent
#-interpreter-exec console set confirm off
#-interpreter-exec console eval "add-symbol-file ../_output/system/%s 0x%lx", module_file_name, entry
#print 1
#-interpreter-exec console c
#-interpreter-exec console end