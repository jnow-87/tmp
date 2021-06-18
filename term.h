/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_TERM_H
#define DRIVER_TERM_H


#include <stdint.h>


/* types */
typedef uint8_t int_num_t;

typedef enum{
	TERR_NONE = 0x0,
	TERR_DATA_OVERRUN = 0x1,
	TERR_PARITY = 0x2,
	TERR_FRAME = 0x4,
	TERR_RX_FULL = 0x8,
	TERR_WRITE_COLL = 0x10,
} term_err_t;

typedef enum{
	TIFL_CRNL = 0x1,
	TIFL_NLCR = 0x2,
} term_iflags_t;

typedef enum{
	TOFL_CRNL = 0x1,
	TOFL_NLCR = 0x2,
} term_oflags_t;

typedef enum{
	TLFL_ECHO = 0x1,
} term_lflags_t;

typedef struct{
	term_iflags_t iflags;
	term_oflags_t oflags;
	term_lflags_t lflags;
} term_flags_t;

typedef struct{
	int (*configure)(void *cfg, void *data);

	term_flags_t *(*get_flags)(void *cfg);

	char (*putc)(char c, void *data);
	size_t (*puts)(char const *s, size_t n, void *data);
	size_t (*gets)(char *s, size_t n, term_err_t *err, void *data);

	void *data;
	int_num_t rx_int,
			  tx_int;

	uint8_t cfg_size;
} term_itf_t;


#endif // DRIVER_TERM_H
