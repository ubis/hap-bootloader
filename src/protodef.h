/*
 *		      HAP-Bootoader, based on OpenBLT
 *
 *	@author		Jokubas Maciulaitis (ubis)
 *	@file		protodef.c
 *	@date		2019 05 11
 */

#ifndef PROTODEF_H
#define PROTODEF_H

typedef enum {
	MODE_BOOT,
} mode_t;

typedef enum {
	BOOT_FAIL_CHECKSUM,
	BOOT_FAIL_PRE_START,
	BOOT_FAIL_START,
} boot_fail_t;

typedef enum {
	ID_11_BIT,
	ID_29_BIT = 0x04,
} identifier_t;

typedef union {
	struct {
		int address    : 16;
		int uid_part   : 8;
		int reserved   : 4;
		int mode       : 1;
		int identifier : 3;
	};

	int id;
} header_t;

#endif