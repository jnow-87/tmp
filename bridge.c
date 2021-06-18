/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdint.h>
#include <string.h>
#include "termbridge.h"


/* global functions */
void term_bridge_init(term_bridge_t *brdg, size_t id, term_itf_t *term, uint8_t chunksize_e){
	memset(brdg, 0, sizeof(term_bridge_t));

	brdg->id = id;
	brdg->chunksize_e = chunksize_e;
	brdg->term = term;
}
