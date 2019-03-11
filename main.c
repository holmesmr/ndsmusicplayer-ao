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

#include "ao/ao.h"
#include "ao.h"
#include "libao.h"
#include "corlett.h"
#include "vio2sf/vio2sf.h"
#include "color.h"
#include "xsf_player.h"
#include "common.h"

char *xsf_tagget(const char *tag, const char *pData, int dwSize);

// load and set up a 2sf file
int setup_playback(char *name)
{
	int res;

	res = read_in_file(name);

	if (res < 0) {
		return res;
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

int main(int argc, char *argv[])
{
	struct termios tp;
	struct timeval tv;
	int fds;
	fd_set watchset;
	char ch = 0;
	int song;

	MAGENTA();
	printf("NDS Music Player, libao edition\n");
	printf("Version 4.0\n");
	printf("Using vio2sf 0.15\n");
	NORMAL();
	// check if an argument was given
	if (argc < 2)
	{
		RED();
		printf("Error: must specify a filename or names!\n");
		NORMAL();
		return -1;
	}

	GREEN();
	printf("Press ESC or Q to stop. p = previous song, n = next song\n\n", argv[1]);
	NORMAL();

	if (setup_playback(argv[1]) < 0)
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
		if ((ch == 'n') && ((song+1) < argc))
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

			if (setup_playback(argv[song]) < 0)
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

			if (setup_playback(argv[song]) < 0)
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
