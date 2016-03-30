OUTPUT_FORMAT("binary")
ENTRY(_start)
STARTUP("long_mode_main.o")
SECTIONS
{
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

}