rem "C:\temp\realtemp\vmrun.exe.lnk" -T player start "E:\vms\MS-DOS\MS-DOS.vmx"
start E:\vms\MS-DOS\MS-DOS.vmx
python.exe wait_for_con.py
python.exe gdb_proxy.py %1 %2