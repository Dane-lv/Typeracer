#include <SDL3_mixer/SDL_mixer.h>
#include "stdlib.h"
#include "audio.h"


struct audio{
    Mix_Chunk *pClickSound;
    Mix_Chunk *pBgMusic;
    Mix_Chunk *pCountDown;
    Mix_Chunk *pCountDown_finish;
};


Audio *createAudio(){
    Audio *pAudio = malloc(sizeof(struct audio));
    if (!SDL_Init(SDL_INIT_AUDIO)){ printf("Error SDL Init: %s\n", SDL_GetError()); return NULL;}
    if (!Mix_OpenAudio(0, NULL)) {
        SDL_Log("Mix_OpenAudio failed: %s", SDL_GetError());
        return NULL;
    }
    Mix_MasterVolume(MIX_MAX_VOLUME / 12); // lower the volume 
    pAudio->pClickSound = Mix_LoadWAV("lib/resources/clicksound.wav");
    pAudio->pBgMusic = Mix_LoadWAV("lib/resources/bgmusic.wav");
    pAudio->pCountDown = Mix_LoadWAV("lib/resources/countdown.wav");
    pAudio->pCountDown_finish = Mix_LoadWAV("lib/resources/countdownfinish.wav");
    return pAudio;
}

void playCountdownFinish(Audio *pAudio){
    Mix_PlayChannel(-1, pAudio->pCountDown_finish, 0);
}

void playCountdown(Audio *pAudio){

    Mix_PlayChannel(-1, pAudio->pCountDown, 0);
    
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
    if(pAudio->pCountDown) Mix_FreeChunk(pAudio->pCountDown);
    if(pAudio->pClickSound) Mix_FreeChunk(pAudio->pClickSound);
    if(pAudio->pBgMusic) Mix_FreeChunk(pAudio->pBgMusic);
    free(pAudio);
}