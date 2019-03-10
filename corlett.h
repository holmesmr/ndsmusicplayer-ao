//
// Audio Overload
// Emulated music player
//
// (C) 2000-2008 Richard F. Bannister
//

// corlett.h

#include "ao.h"

#define MAX_UNKNOWN_TAGS			32

typedef struct {
	char lib[256];
	char libaux[8][256];
	
	char inf_title[256];
	char inf_copy[256];
	char inf_artist[256];
	char inf_game[256];
	char inf_year[256];
	char inf_length[256];
	char inf_fade[256];

	char inf_refresh[256];
	
	char tag_name[MAX_UNKNOWN_TAGS][256];
	char tag_data[MAX_UNKNOWN_TAGS][256];

	UINT32 *res_section;
    UINT32 res_size;
} corlett_t;

int corlett_decode(UINT8 *input, UINT32 input_len, UINT8 **output, UINT64 *size, corlett_t **c);
UINT32 psfTimeToMS(char *str);

