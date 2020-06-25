/*
 * linkerscript
 */

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
/*STARTUP(start.o)*/
/*SEARCH_DIR(.)*/
/*__DYNAMIC = 0;*/

MEMORY
{
	flash : ORIGIN = 0x08000000, LENGTH = 16M
	ram : ORIGIN = 0x02200000, LENGTH = 1M
}

SECTIONS
{
	.flash :
	{
		*(.text)
	} > flash

	_ram_load = ADDR(.flash) + SIZEOF(.flash);
	.ram : AT(_ram_load)
	{
		*(.text2)
		*(.data)
		*(.bss)
	} > ram
	PROVIDE(_ram_addr = ADDR(.ram));
	PROVIDE(_ram_size = SIZEOF(.ram));
}
