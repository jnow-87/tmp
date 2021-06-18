#define _GNU_SOURCE
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "termbridge.h"


/* macros */
#define CHUNKSIZE_E	1


/* local/static prototypes */
static void user(int term, term_bridge_t *brdg);
static void help(void);

static int tty_init(void);
static int tty_configure(int fd, int iflags, int oflags, int lflags);

static int term_configure(void *cfg, void *data);
static term_flags_t *term_get_flags(void *cfg);
static char term_putc(char c, void *data);
static size_t term_puts(char const *s, size_t n, void *data);
static size_t term_gets(char *s, size_t n, term_err_t *err, void *data);


/* global functions */
int main(void){
	int term;
	term_bridge_t brdg;
	term_itf_t titf;


	/* init terminal */
	term = tty_init();

	if(term == -1)
		return 1;

	if(tty_configure(term, 0, 0, 0) != 0)
		return 2;

	printf(
		"pid: %u\n"
		"tty: %s\n\n"
		,
		getpid(),
		ptsname(term)
	);

	help();

	/* init terminal bridge */
	titf.configure = term_configure;
	titf.get_flags = term_get_flags;
	titf.putc = term_putc;
	titf.puts = term_puts;
	titf.gets = term_gets;
	titf.data = &term;
	titf.rx_int = 0;
	titf.tx_int = 0;
	titf.cfg_size = 0;

	term_bridge_init(&brdg, 0, &titf, CHUNKSIZE_E);

	/* main */
	user(term, &brdg);

	/* cleanup */
	close(term);

	return 0;
}


/* local functions */
static void user(int term, term_bridge_t *brdg){
	char *line;
	char msg[64];
	ssize_t n;
	size_t x;


	read_history(".brdg.hst");

	while(1){
		line = readline("#: ");
		n = strlen(line);

		if(n == 0)
			continue;

		// read
		if(strncmp(line, "r", 1) == 0){
			x = atol(line + 2);

			printf("read %zu bytes\n", x);

			n = read(term, msg, x);
			msg[n] = 0;

			printf("read: %zd \"%s\"\n", n, (n > 0) ? msg : "");
		}
		// write
		else if(strncmp(line, "w", 1) == 0){
			n = write(term, line + 2, n - 2);
			printf("written: %zd\n", n);
		}
		// bridge read
		else if(strncmp(line, "br", 2) == 0){
			x = atol(line + 2);

			printf("bridge read %zu bytes\n", x);

			n = term_bridge_read(brdg, msg, x);
			msg[n] = 0;

			printf("read: %zd \"%s\"\n", n, (n > 0) ? msg : "");
		}
		// bridge write
		else if(strncmp(line, "bw", 2) == 0){
			n = term_bridge_write(brdg, line + 3, n - 3);
			printf("written: %zd\n", n);
		}
		// help
		else if(strcmp(line, "h") == 0){
			help();
		}
		// quit
		else if(strcmp(line, "q") == 0){
			break;
		}

		add_history(line);
	}

	write_history(".brdg.hst");
}

static void help(void){
	printf(
		"commands:\n"
		"%20.20s    %s\n"
		"%20.20s    %s\n"
		"%20.20s    %s\n"
		"%20.20s    %s\n"
		"%20.20s    %s\n"
		"%20.20s    %s\n"
		, "r <nbytes>", "read <nbytes> from tty"
		, "w <string>", "write <string> to tty"
		, "br <nbytes>", "read <nbytes> from bridge"
		, "bw <string>", "write <string> to bridge"
		, "h", "print this help message"
		, "q", "quit"
	);
}

static int tty_init(void){
	int fd;


	fd = getpt();

	if(fd == -1)
		return -1;

	if(grantpt(fd) != 0 || unlockpt(fd) != 0)
		return -1;

	return fd;
}

static int tty_configure(int fd, int iflags, int oflags, int lflags){
	struct termios cfg;


	if(tcgetattr(fd, &cfg) != 0)
		return -1;

	cfg.c_iflag = iflags;
	cfg.c_oflag = oflags;
	cfg.c_lflag = lflags;

	if(tcsetattr(fd, TCSANOW, &cfg) != 0)
		return -1;

	return 0;
}

static int term_configure(void *cfg, void *data){
	printf("term_configure\n");
	return 0;
}

static term_flags_t *term_get_flags(void *cfg){
	static term_flags_t flags = { 0 };

	return &flags;
}

static char term_putc(char c, void *data){
	if(term_puts(&c, 1, data) == 1)
		return c;
	return ~c;
}

static size_t term_puts(char const *s, size_t n, void *data){
	return write(*((int*)data), s, n);
}

static size_t term_gets(char *s, size_t n, term_err_t *err, void *data){
	*err = TERR_NONE;

	return read(*((int*)data), s, n);
}
