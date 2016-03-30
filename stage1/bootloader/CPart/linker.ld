OUTPUT_FORMAT("binary")
ENTRY(_start)
SECTIONS
{
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

}