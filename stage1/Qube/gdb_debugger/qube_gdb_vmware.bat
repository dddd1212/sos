"C:\Program Files (x86)\VMware\VMware VIX\vmrun.exe" -T player start "E:\vms\MS-DOS\MS-DOS.vmx"
sleep 1
python.exe gdb_proxy.py %1 %2