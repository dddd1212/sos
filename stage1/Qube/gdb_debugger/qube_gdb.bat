start "" "%BOCHS%\bochs_gdb.exe" -q -f bochsrc_gdb.bxrc
rem gdb %1 %2
rem start python gdb_proxy.py %1 %2
rem sleep 5
python.exe gdb_proxy.py %1 %2