//
// Created by cepheus on 3/10/19.
//

#include "xsf_player.h"

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "color.h"
#include "common.h"
#include "vio2sf/vio2sf.h"

static uint32 size;   // size of buffer

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

extern void do_frame(uint32 size, int16 *buffer)
{
    xsf_gen(buffer, size);
}

// load and set up a 2sf file
int read_in_file(char *name) {
    FILE *file;
    uint32 filesig;
    uint8 *filedata;
    uint64 file_len;
    unsigned int size;
    struct stat st;

    c = NULL;

    file = fopen(name, "rb");

    if (!file) {
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

    size = (unsigned int) st.st_size;

    buffer = malloc((size_t) size);

    if (!buffer) {
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
    if (corlett_decode(buffer, size, &filedata, &file_len, &c) != AO_SUCCESS) {
        RED();
        printf("ERROR: Tag format unreadable in file ");
        MAGENTA();
        printf("%s\n", name);
        NORMAL();
        return -1;
    }
    free(filedata);    // we don't use this

    if (xsf_start(buffer, size) != XSF_TRUE) {
        RED();
        printf("ERROR: vio2sf failed to load file \n");
        MAGENTA();
        printf("%s\n", name);
        NORMAL();
        return -1;
    }

    return 0;
}