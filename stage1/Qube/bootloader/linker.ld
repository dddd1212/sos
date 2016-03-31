OUTPUT_FORMAT("binary")
ENTRY(real_mode)
STARTUP("real_mode.o")
SECTIONS
{
	.text 0x7c00:
	{
		*(.text)
	}

}