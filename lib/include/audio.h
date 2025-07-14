#ifndef audio_h
#define audio_h

typedef struct audio Audio;

Audio *createAudio();
void playClickSound(Audio *pAudio);
void playBgMusic(Audio *pAudio);
void destroyAudio(Audio *pAudio);

void destroyBgMusic(Audio *pAudio); // when game starts 

#endif