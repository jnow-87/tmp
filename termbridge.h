/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_TERMBRIDGE_H
#define DRIVER_TERMBRIDGE_H


#include <stdint.h>
#include "term.h"


/* types */
typedef struct term_bridge_t{
	struct term_bridge_t *prev,
						 *next;

	size_t id;
	uint8_t chunksize_e;

	uint8_t seq_num;

	term_itf_t *term;
	term_err_t terr;
} term_bridge_t;


/* prototypes */
void term_bridge_init(term_bridge_t *brdg, size_t id, term_itf_t *term, uint8_t chunksize_e);

int16_t term_bridge_read(term_bridge_t *brdg, void *data, uint8_t n);
int16_t term_bridge_write(term_bridge_t *brdg, void const *data, uint8_t n);


#endif // DRIVER_TERMBRIDGE_H
