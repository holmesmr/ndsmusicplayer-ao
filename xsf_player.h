#ifndef _XSF_PLAYER_H_

#define _XSF_PLAYER_H_

#include "ao.h"
#include "corlett.h"

corlett_t *c;
uint8 *buffer; // buffer containing 2sf file

int xsf_get_lib(char *filename, void **buffer, unsigned int *length);
extern void do_frame(uint32 size, int16 *buffer);
int read_in_file(char *name);

#endif //_XSF_PLAYER_H_
