/*
 *		      HAP-Bootoader, based on OpenBLT
 *
 *	@author		Jokubas Maciulaitis (ubis)
 *	@file		flash_layout.c
 *	@date		2019 04 22
 */

/* Array wit the layout of the flash memory.
 *  Also controls what part of the flash memory is reserved for the bootloader.
 *  If the bootloader size changes, the reserved sectors for the bootloader
 *   might need adjustment to make sure the bootloader doesn't get overwritten.
 */

/* space is reserved for a bootloader configuration with all supported
 * communication interfaces enabled. when for example only UART is needed,
 * than the space required for the bootloader can be made a lot smaller here.
 */
static const tFlashSector flashLayout[] =
{
	// { 0x08000000, 0x02000,  0}, flash sector  0 - reserved for bootloader

	{ 0x08002000, 0x02000,  1}, // flash sector  1 - 8kb
	{ 0x08004000, 0x02000,  2}, // flash sector  2 - 8kb
	{ 0x08006000, 0x02000,  3}, // flash sector  3 - 8kb
	{ 0x08008000, 0x02000,  4}, // flash sector  4 - 8kb
	{ 0x0800A000, 0x02000,  5}, // flash sector  5 - 8kb
	{ 0x0800C000, 0x02000,  6}, // flash sector  6 - 8kb
	{ 0x0800E000, 0x02000,  7}, // flash sector  7 - 8kb
	{ 0x08010000, 0x02000,  8}, // flash sector  8 - 8kb
	{ 0x08012000, 0x02000,  9}, // flash sector  9 - 8kb
	{ 0x08014000, 0x02000, 10}, // flash sector 10 - 8kb
	{ 0x08016000, 0x02000, 11}, // flash sector 11 - 8kb
	{ 0x08018000, 0x02000, 12}, // flash sector 12 - 8kb
	{ 0x0801A000, 0x02000, 13}, // flash sector 13 - 8kb
	{ 0x0801C000, 0x02000, 14}, // flash sector 14 - 8kb
	{ 0x0801E000, 0x02000, 15}, // flash sector 15 - 8kb
};
