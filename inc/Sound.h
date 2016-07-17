
#include "SDL.h"

#ifndef SOUND_H
#define SOUND_H

typedef struct AUD_sSound{
    SDL_AudioSpec spec;
    int loop, volume;
    SDL_AudioDeviceID dev;
    Uint8* buf, *cur;
    Uint32 buflen, curlen;
} AUD_Sound;


AUD_Sound* AUD_LoadWAV(const char* wav, int loop);
void AUD_FreeWav(AUD_Sound* sound);


void AUD_Play(AUD_Sound* sound);


#endif // Sound
