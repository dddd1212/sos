ENTRY(real_mode)
STARTUP("../_output/bootloader/inter/real_mode.o")
SECTIONS
{
	.text 0x7c00:
	{
		*(.text)
	}

}
