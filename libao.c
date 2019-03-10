/*

	Audio Overload SDK

	Copyright (c) 2007, R. Belmont and Richard Bannister.

	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the names of R. Belmont and Richard Bannister nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "ao.h"
#include "libao.h"

#define LOG_WAVE 	(0)
#define VALGRIND 	(0)

#define NUM_FRAGS_BROKEN    (8)
#define NUM_FRAGS_NORMAL    (4)
static INT32 num_frags;
#define OSS_FRAGMENT (0x000D | (num_frags<<16));  // 16k fragments (2 * 2^14).

// local variables

void  (*m1sdr_Callback)(unsigned long dwNumSamples, signed short *data);
unsigned long cbUserData;

static int hw_present;

static INT32 is_broken_driver;
int nDSoundSegLen = 0;
int oss_nw = 0;

ao_device* audiodev = NULL;
#if LOG_WAVE
FILE *logfil;
#endif

static int playtime = 0;

static INT16 samples[44100*2];


// set # of samples per update

void m1sdr_SetSamplesPerTick(UINT32 spf)
{
	nDSoundSegLen = spf;
}

// m1sdr_Update - timer callback routine: runs sequencer and mixes sound

void m1sdr_Update(void)
{	
	if (!hw_present) return;

	if (m1sdr_Callback)
	{
		m1sdr_Callback(nDSoundSegLen, (INT16 *)samples);
	}
}
// checks the play position to see if we should trigger another update

void m1sdr_TimeCheck(void)
{
#if VALGRIND
	m1sdr_Update();
#else
//	audio_buf_info info;

	m1sdr_Update();
	playtime++;

	// output the generated samples
	ao_play(audiodev, samples, nDSoundSegLen * 4);

	usleep(50);
#endif
}

// m1sdr_Init - inits the output device and our global state

INT16 m1sdr_Init(int sample_rate)
{	
	int format, stereo, rate, fsize, driver;
	struct ao_sample_format samp_fmt;

	hw_present = 0;

	nDSoundSegLen = sample_rate / 60;

	memset(samples, 0, 44100*4);	// zero out samples

	m1sdr_Callback = NULL;

	// Init libao
	ao_initialize();

	// Get default driver
	driver = ao_default_driver_id();

	// Open driver
	samp_fmt = (ao_sample_format){
		.bits = 16,
		.rate = sample_rate,
		.channels = 2,
		.byte_format = AO_FMT_NATIVE,
		.matrix = NULL
	};

	audiodev = ao_open_live(driver, &samp_fmt, NULL);

	hw_present = 1;

#if LOG_WAVE
	logfil = fopen("log.bin", "wb");
#endif

	return (1);
}

void m1sdr_Exit(void)
{	
	if (!hw_present || !audiodev) return;

	ao_close(audiodev);
	ao_shutdown();
#if LOG_WAVE
	fclose(logfil);
#endif
}

void m1sdr_SetCallback(void *fn)
{
	if (fn == (void *)NULL)
	{
		printf("ERROR: NULL CALLBACK!\n");
	}

//	printf("m1sdr_SetCallback: aok!\n");
	m1sdr_Callback = (void (*)(unsigned long, signed short *))fn;
}

INT16 m1sdr_IsThere(void)
{
	return audiodev != NULL;
}

INT32 m1sdr_HwPresent(void)
{
	return hw_present;
}

// unused stubs for this driver, but the Win32 driver needs them
void m1sdr_PlayStart(void)
{
	playtime = 0;
}

void m1sdr_PlayStop(void)
{
}

void m1sdr_FlushAudio(void)
{
}

void m1sdr_SetNoWait(int nw)
{
}

short *m1sdr_GetSamples(void)
{
	return samples;
}

int m1sdr_GetPlayTime(void)
{
	return playtime;
}

