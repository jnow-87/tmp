/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdint.h>
#include <stdio.h>
#include "termbridge.h"


/* macros */
#define DEBUG(fmt, ...)	printf(fmt, ##__VA_ARGS__)
//#define DEBUG(fmt, ...)	{}

#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define CHUNKS(n, cs)({ \
	uint8_t x; \
	\
	\
	x = (n) / (cs); \
	x += ((x * (cs)) == n) ? 0 : 1; \
	\
	x; \
})


/* local/static prototypes */
static int read(term_bridge_t *brdg, uint8_t *data, uint8_t n, uint8_t *expect);
static int write(term_bridge_t *brdg, uint8_t const *data, uint8_t n);

static int ack(term_bridge_t *brdg, uint8_t byte);
static int nack(term_bridge_t *brdg, uint8_t byte);

static uint8_t checksum(uint8_t const *data, size_t n);


/* global functions */
int16_t term_bridge_read(term_bridge_t *brdg, void *data, uint8_t n){
	uint8_t i,
			b,
			chunks,
			csum;


	// read control byte
	if(read(brdg, &b, 1, (uint8_t []){ brdg->chunksize }) != 0)
		return -1;

	DEBUG("control byte %#hhx\n", b);

	// read data length
	if(read(brdg, &b, 1, 0x0) != 0 || b > n)
		return -1;

	n = b;
	DEBUG("len %#hhx\n", n);

	// read checksum
	if(read(brdg, &csum, 1, 0x0) != 0)
		return -1;

	DEBUG("checksum %#hhx\n", csum);

	// read data
	chunks = CHUNKS(n, brdg->chunksize);
	DEBUG("payload: cs %u, chunks %u\n", brdg->chunksize, chunks);

	for(i=0; i<chunks; i++){
		if(read(brdg, data + i * brdg->chunksize, MIN(brdg->chunksize, n - (i * brdg->chunksize)), 0x0) != 0)
			return -1;

		for(uint8_t j=0; j<MIN(brdg->chunksize, n - (i * brdg->chunksize)); j++)
			DEBUG("data %#hhx\n", ((char*)data)[i * brdg->chunksize + j]);
	}

	// verify and acknowledge checksum
	b = checksum(data, n);
	DEBUG("verify %#hhx\n", b);

	if(write(brdg, &b, 1) != 0 || b != csum)
		return -1;

	DEBUG("read complete\n");

	return n;
}

int16_t term_bridge_write(term_bridge_t *brdg, void const *data, uint8_t n){
	uint8_t i,
			chunks,
			csum;


	// write control byte
	DEBUG("control byte %#hhx\n", brdg->chunksize);

	if(write(brdg, (uint8_t []){ brdg->chunksize }, 1) != 0)
		return -1;

	// write data length
	DEBUG("len %#hhx\n", n);

	if(write(brdg, &n, 1) != 0)
		return -1;

	// write checksum
	csum = checksum(data, n);
	DEBUG("checksum %#hhx\n", csum);

	if(write(brdg, &csum, 1) != 0)
		return -1;

	// write data
	chunks = CHUNKS(n, brdg->chunksize);
	DEBUG("payload: cs %u, chunks %u\n", brdg->chunksize, chunks);

	for(i=0; i<chunks; i++){
		for(uint8_t j=0; j<MIN(brdg->chunksize, n - (i * brdg->chunksize)); j++)
			DEBUG("data %#hhx\n", ((char*)data)[i * brdg->chunksize + j]);

		if(write(brdg, data + i * brdg->chunksize, MIN(brdg->chunksize, n - (i * brdg->chunksize))) != 0)
			return -1;
	}

	// read acknowledge
	DEBUG("verify %#hhx\n", csum);

	if(read(brdg, &i, 1, &csum) != 0)
		return -1;

	DEBUG("write complete\n");

	return n;
}


/* local functions */
static uint8_t checksum(uint8_t const *data, size_t n){
	uint8_t s;
	size_t i;


	s = 0;

	for(i=0; i<n; i++)
		s += ~data[i] + 1;

	return s;
}

static int read(term_bridge_t *brdg, uint8_t *data, uint8_t n, uint8_t *expect){
	size_t i,
		   r;
	term_itf_t *term;


	term = brdg->term;

	for(i=0; i<n; i+=r){
		r = term->gets((char*)data + i, n - i, &brdg->terr, term->data);

		if(r == 0 || brdg->terr){
			DEBUG("read error\n");
			return nack(brdg, data[i]);
		}

		DEBUG("read n=%u, %#hhx/~%#hhx\n", r, data[i], ~data[i]);
	}

	if(expect != 0x0){
		for(i=0; i<n; i++){
			if(data[i] != expect[i]){
				DEBUG("expect error\n");
				return nack(brdg, data[n - 1]);
			}
		}
	}

	return ack(brdg, data[n - 1]);
}

static int write(term_bridge_t *brdg, uint8_t const *data, uint8_t n){
	uint8_t c;
	size_t r;
	term_itf_t *term;


	term = brdg->term;

	DEBUG("write n=%u, %#hhx/~%#hhx\n", n, data[0], ~data[0]);
	r = term->puts((char const*)data, n, term->data);

	if(term->gets((char*)&c, 1, &brdg->terr, term->data) != 1 || brdg->terr){
		DEBUG("read ack failed\n");
		return -1;
	}

	DEBUG("read %#hhx/~%#hhx\n", c, ~c);
	c = ~c;

	if(r != n || c != data[n - 1]){
		DEBUG("ack failed: len %#hhx ?= %#hhx, data %#hhx ?= %#hhx\n", (uint8_t)r, n, c, data[n - 1]);
		return -1;
	}

	return 0;
}

static int ack(term_bridge_t *brdg, uint8_t byte){
	uint8_t x;

	DEBUG("write ack %#hhx/~%#hhx\n", ~byte, byte);
	x = brdg->term->putc(~byte, brdg->term->data);

	if(x == (uint8_t)~byte){
		DEBUG("ack success %#hhx/~%#hhx\n", byte, ~byte);
		return 0;
	}

	DEBUG("ack failed %#hhx != %#hhx\n", x, ~byte);
	return -1;
//	return brdg->term->putc(~byte, brdg->term->data) == ~byte ? 0 : -1;
}

static int nack(term_bridge_t *brdg, uint8_t byte){
	DEBUG("nack\n");
	(void)ack(brdg, ~byte);

	return -1;
}
