#ifndef game_h
#define game_h

typedef struct gameCore GameCore;

GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height);

GameCoreData *getGData_local(GameCore *pCore);
void createNames(GameCore *pCore);
void destroyGameCore(GameCore *pCore);
void renderNames(GameCore *pCore);
void renderCore(GameCore *pCore);
int readFromFile(GameCore *pCore);
void renderRectangle(GameCore *pCore);
int gameCoreInputHandle(GameCore *pCore, SDL_Event *event);
void renderInput(GameCore *pCore);

#endif