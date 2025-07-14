#include <SDL3_mixer/SDL_mixer.h>
#include "stdlib.h"
#include "audio.h"


struct audio{
    Mix_Chunk *pClickSound;
    Mix_Chunk *pBgMusic;
};


Audio *createAudio(){
    Audio *pAudio = malloc(sizeof(struct audio));
    if (!SDL_Init(SDL_INIT_AUDIO)){ printf("Error SDL Init: %s\n", SDL_GetError()); return NULL;}
    if (!Mix_OpenAudio(0, NULL)) {
        SDL_Log("Mix_OpenAudio failed: %s", SDL_GetError());
        return NULL;
    }
    pAudio->pClickSound = Mix_LoadWAV("lib/resources/clicksound.wav");
    pAudio->pBgMusic = Mix_LoadWAV("lib/resources/bgmusic.wav");
    return pAudio;
}

void playClickSound(Audio *pAudio){
    Mix_PlayChannel(-1, pAudio->pClickSound, 0);
}

void playBgMusic(Audio *pAudio){
    Mix_PlayChannel(-1, pAudio->pBgMusic, -1);
}

void destroyBgMusic(Audio *pAudio){
    Mix_FreeChunk(pAudio->pBgMusic);
    pAudio->pBgMusic = NULL;
}



void destroyAudio(Audio *pAudio){
    if(pAudio->pClickSound) Mix_FreeChunk(pAudio->pClickSound);
    if(pAudio->pBgMusic) Mix_FreeChunk(pAudio->pBgMusic);
    free(pAudio);
}