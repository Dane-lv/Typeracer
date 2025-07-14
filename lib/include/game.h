#ifndef game_h
#define game_h

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "stateAndData.h"

typedef struct audio Audio;
typedef struct gameCore GameCore;

GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height, int textToLoad, Audio *pAudio);

GameCoreData *getGData_local(GameCore *pCore);
void createNamesAndWPM(GameCore *pCore);
void destroyGameCore(GameCore *pCore);
void renderNames(GameCore *pCore);
void renderCore(GameCore *pCore, Audio *pAudio);
int readFromFile(GameCore *pCore, int textToLoad);
void renderRectangle(GameCore *pCore);
int gameCoreInputHandle(GameCore *pCore, SDL_Event *event);
void renderInput(GameCore *pCore);
void parseText(GameCore *pCore);
void checkInput(GameCore *pCore);
void createTextAsWords(GameCore *pCore);
void renderText(GameCore *pCore);
void updateCursorPosition(GameCore *pCore);
void setWordGreen(GameCore *pCore);
void updateGameCore(GameCore *pCore);
char *getWPM(GameCore *pCore);
void setGameCoreChanged(GameCore *pCore, bool changed);
void updateCars(GameCore *pCore);
int getCurrentWordIndex(GameCore *pCore);
void checkSpelling(GameCore *pCore);
void renderHighlightRectangle(GameCore *pCore);
void renderCountdown(GameCore *pCore, Audio *pAudio);
#endif