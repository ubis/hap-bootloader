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
	COMMAND_BOOT_PERFORM,
	COMMAND_BOOT_INIT,
	COMMAND_BOOT_PREPARE_LAUNCH,
	COMMAND_BOOT_FAIL,
	COMMAND_BOOT_GET_STATE,
	COMMAND_BOOT_GET_VERSION,
} command_t;

typedef enum {
	STATE_BOOTLOADER,
	STATE_APPLICATION,
} state_t;

typedef enum {
	BOOT_FAIL_CHECKSUM,
	BOOT_FAIL_PRE_START,
	BOOT_FAIL_START,
} boot_fail_t;

typedef union {
	struct {
		int mode     : 1;
		int reserved : 2;
		int type     : 2;
		int command  : 8;
		int group    : 8;
		int address  : 8;
	};

	int id;
} header_t;

#endif