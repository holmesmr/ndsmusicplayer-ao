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
#include <time.h>

#include "ao.h"
#include "libao.h"
#include "corlett.h"
#include "vio2sf/vio2sf.h"
#include "color.h"
#include "xsf_player.h"
#include "common.h"

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

char *xsf_tagget(const char *tag, const char *pData, int dwSize);

#define FILE_BUF_LEN 256

static struct ao_sample_format samp_fmt = {
        .bits = 16,
        .rate = 44100,
        .channels = 2,
        .byte_format = AO_FMT_NATIVE,
        .matrix = NULL
};

int main(int argc, char *argv[])
{
    struct termios tp;
    struct timeval tv;
    int fds;
    fd_set watchset;
    char ch = 0;
    int song;
    int driver;
    // 16 bit (2 byte) depth * 2 chans * 44100 samples/sec
    unsigned char* samples[44100*2*2];

    MAGENTA();
    printf("NDS Music Dumper, libao edition\n");
    printf("Version 0.1\n");
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

    ao_initialize();
    driver = ao_driver_id("wav");

    if (driver == -1) {
        RED();
        printf("Error: built libao does not support WAV output!\n");
        NORMAL();
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        char* suffix_ptr = 0;
        size_t pos = 0;
        char new_filename[FILE_BUF_LEN] = { 0 };
        clock_t start, end;
        double cpu_time_used;
        ao_device* fdev;
        int mins = 0, secs = 0;
        start = clock();

        memset(samples, 0, 44100*4);	// zero out samples

        if (read_in_file(argv[i]) < 0)
        {
            return -1;
        }

        suffix_ptr = strstr(argv[i], ".mini2sf");

        if (!suffix_ptr) {
            suffix_ptr = strstr(argv[i], ".2sf");
        }

        if (suffix_ptr) {
            strncpy(new_filename, argv[i], min(suffix_ptr - argv[i], FILE_BUF_LEN - 5));
        } else {
            strncpy(new_filename, argv[i], FILE_BUF_LEN - 5);
        }

        pos = strlen(new_filename);

        memcpy(new_filename + pos, ".wav", 5);

        NORMAL();
        printf("Dumping ");
        BOLD();
        printf("%s", argv[i]);
        YELLOW();
        printf(" => ");
        MAGENTA();
        printf("%s [%s]\n", new_filename, c->inf_length);
        sscanf(c->inf_length, "%d:%d", &mins, &secs);
        secs += mins * 60;
        // Just to avoid cutoff
        secs++;

        fdev = ao_open_file(driver, new_filename, 1, &samp_fmt, NULL);

        int i;
        for (i = 0; i < secs; i++) {
            int frame = sizeof(samples) / 8;
            do_frame(frame / 4, (short*)samples);
            ao_play(fdev, samples, frame);
        }
        printf("Serialized %d seconds (expected %d)\n", i, secs);

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        printf("Dumped in %lf seconds.\n", cpu_time_used);

        NONBOLDYELLOW();
        printf("----------------------------------------------------------------\n");
        NORMAL();

        ao_close(fdev);
    }


    ao_shutdown();
    xsf_term();

    free(buffer);

    return 0;
}
