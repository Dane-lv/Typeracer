#ifndef game_h
#define game_h

typedef struct gameCore GameCore;

GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height);

GameCoreData *getGData_local(GameCore *pCore);

void destroyGameCore(GameCore *pCore);

#endif