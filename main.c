/*
  Linux 2SF player - main program
  Rewritten by Jesse N. Richardson

  Ported to use LibAO by Michael Holmes
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>

#include "ao.h"
#include "libao.h"
#include "corlett.h"
#include "vio2sf/vio2sf.h"
#include "color.h"

static uint8 *buffer; // buffer containing 2sf file
static uint32 size;   // size of buffer
static corlett_t *c = NULL;

char *xsf_tagget(const char *tag, const char *pData, int dwSize);

#define UINT_MAX (1 << ((sizeof(unsigned int) - 1) * 8))

/* ao_get_lib: called to load secondary files */
int xsf_get_lib(char *filename, void **buffer, unsigned int *length)
{
	uint8 *filebuf;
	unsigned int size;
	FILE *auxfile;
	struct stat st;

	auxfile = fopen(filename, "rb");
	if (!auxfile)
	{
		RED();
		printf("Unable to find auxiliary file %s\n", filename);
		NORMAL();
		return AO_FAIL;
	}

	fstat(fileno(auxfile), &st);

	if (st.st_size > UINT_MAX) {
		fclose(auxfile);
		RED();
		printf("ERROR: file size of %zu bytes is larger than maximum supported value of %u\n", st.st_size, UINT_MAX);
		NORMAL();
		return AO_FAIL;
	}

	size = (unsigned int)st.st_size;

	filebuf = malloc((size_t)size);

	if (!filebuf)
	{
		fclose(auxfile);
		RED();
		printf("ERROR: could not allocate %d bytes of memory\n", size);
		NORMAL();
		return AO_FAIL;
	}

	fread(filebuf, size, 1, auxfile);
	fclose(auxfile);

	*buffer = filebuf;
	*length = size;

	return AO_SUCCESS;
}

static void do_frame(uint32 size, int16 *buffer)
{
	xsf_gen(buffer, size);
}

// load and set up a 2sf file
int load_file(char *name)
{
	FILE *file;
	uint32 filesig;
	uint8 *filedata;
	uint64 file_len;
	unsigned int size;
	struct stat st;

	file = fopen(name, "rb");

	if (!file)
	{
		RED();
		printf("ERROR: could not open file %s\n", name);
		NORMAL();
		return -1;
	}

	// get the length of the file by using fstat
	fstat(fileno(file), &st);

	if (st.st_size > UINT_MAX) {
		fclose(file);
		RED();
		printf("ERROR: file size of %zu bytes is larger than maximum supported value of %u\n", st.st_size, UINT_MAX);
		NORMAL();
		return -1;
	}

	size = (unsigned int)st.st_size;

	buffer = malloc((size_t)size);

	if (!buffer)
	{
		fclose(file);
		RED();
		printf("ERROR: could not allocate %d bytes of memory\n", size);
		NORMAL();
		return -1;
	}

	// read the file
	fread(buffer, size, 1, file);
	fclose(file);

	// init our *SF engine so we can get tags
	if (corlett_decode(buffer, size, &filedata, &file_len, &c) != AO_SUCCESS)
	{
		RED();
		printf("ERROR: Tag format unreadable in file ");
		MAGENTA();
		printf("%s\n", name);
		NORMAL();
		return -1;
	}
	free(filedata);	// we don't use this

	if (xsf_start(buffer, size) != XSF_TRUE)
	{
		RED();
		printf("ERROR: vio2sf failed to load file \n");
		MAGENTA();
		printf("%s\n", name);
		NORMAL();
		return -1;
	}

	m1sdr_Init(44100);
	m1sdr_SetCallback(do_frame);
	m1sdr_PlayStart();

	if ((c != NULL) && (c->inf_title[0] != '\0'))
	{
		BOLD();
		//printf("Playing \"%s\" by %s from %s.  Copyright %s %s.\nFilename: %s\n", c->inf_title, c->inf_artist, c->inf_game, c->inf_copy, c->inf_year, name);
		printf("Playing \"%s\" by %s from %s.\n", c->inf_title, c->inf_artist, c->inf_game);
		printf("Copyright %s %s.\n", c->inf_copy, c->inf_year);
		printf("Filename: %s.\n", name);
		NONBOLDYELLOW();
		printf("----------------------------------------------------------------\n");
		NORMAL();
	}
	else
	{
		BOLD();
		printf("Playing %s\n", name);
		NORMAL();
	}

	return 0;
}

int main(int argv, char *argc[])
{
	struct termios tp;
	struct timeval tv;
	int fds;
	fd_set watchset;
	char ch = 0;
	int song;

	MAGENTA();
	printf("NDS Music Player, libao edition\n");
	printf("Version 3.8\n");
	printf("Using vio2sf 0.15\n");
	NORMAL();
	// check if an argument was given
	if (argv < 2)
	{
		RED();
		printf("Error: must specify a filename or names!\n");
		NORMAL();
		return -1;
	}

	GREEN();
	printf("Press ESC or Q to stop. p = previous song, n = next song\n\n", argc[1]);
	NORMAL();

	if (load_file(argc[1]) < 0)
	{
		return -1;
	}

	tcgetattr(STDIN_FILENO, &tp);
	tp.c_lflag &= ~ICANON;
	tp.c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
	tcsetattr(STDIN_FILENO, TCSANOW, &tp);

	ch = 0;
	song = 1;
	while ((ch != 27) && (ch != 'q') && (ch != 'Q'))
	{
		fds = STDIN_FILENO;
		FD_ZERO(&watchset);
		FD_SET(fds, &watchset);
		tv.tv_sec = 0;
		tv.tv_usec = 16666/2;	// timeout every 1/120th of a second
		if (select(fds+1, &watchset, NULL, NULL, &tv))
		{
			ch = getchar();	// (blocks until something is pressed)
		}
		else
		{
			ch = 0;
		}

		m1sdr_TimeCheck();

		// Added the ability to press the n key to goto the next song
		if ((ch == 'n') && ((song+1) < argv))
		{
			xsf_term();
			m1sdr_Exit();
			if (c)
			{
				free(c);
				c = NULL;
			}
			free(buffer);
			song++;

			if (load_file(argc[song]) < 0)
			{
				ch = 27;
			}
		}

		if ((ch == 'p') && (song > 1))
		{
			xsf_term();
			m1sdr_Exit();
			if (c)
			{
				free(c);
				c = NULL;
			}
			free(buffer);
			song--;

			if (load_file(argc[song]) < 0)
			{
				ch = 27;
			}
		}
	}

	xsf_term();

	tcgetattr(STDIN_FILENO, &tp);
	tp.c_lflag |= ICANON;
	tp.c_lflag |= (ECHO | ECHOCTL | ECHONL);
	tcsetattr(STDIN_FILENO, TCSANOW, &tp);

	free(buffer);

	return 1;
}
