start "" "%BOCHSGDB%" -q -f bochsrc_gdb.bxrc

python.exe gdb_proxy.py %1 %2