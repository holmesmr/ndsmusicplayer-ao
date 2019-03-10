#ifndef _LIBAO_H_
#define _LIBAO_H_

#include <ao/ao.h>
#include <stdint.h>

extern ao_device* audiodev;
extern void (*m1sdr_Callback)(unsigned long dwUser, signed short *smp);
extern unsigned long cbUserData;

// function protos

void m1sdr_Update(void);
int16_t m1sdr_Init(int sample_rate);
void m1sdr_Exit(void);
void m1sdr_PlayStart(void);
void m1sdr_PlayStop(void);
int16_t m1sdr_IsThere(void);
void m1sdr_TimeCheck(void);
void m1sdr_SetSamplesPerTick(uint32_t spf);
void m1sdr_SetHz(uint32_t hz);
void m1sdr_SetCallback(void *fn);
void m1sdr_SetCPUHog(int hog);
int32_t m1sdr_HwPresent(void);
void m1sdr_FlushAudio(void);
void m1sdr_Pause(int); 
void m1sdr_SetNoWait(int nw);
short *m1sdr_GetSamples(void);
int m1sdr_GetPlayTime(void);
#endif
