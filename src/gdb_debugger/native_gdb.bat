start "" "%BOCHSGDB%" -q -f bochsrc_gdb.bxrc
sleep 1
start gdb --eval-command="target remote localhost:1234"

