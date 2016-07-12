rem start "" "%BOCHSGDB%" -q -f bochsrc_gdb.bxrc
sleep 1
start gdb --eval-command="target remote localhost:1234"

rem start python gdb_proxy.py %1 %2
rem sleep 5
rem python.exe gdb_proxy.py %1 %2