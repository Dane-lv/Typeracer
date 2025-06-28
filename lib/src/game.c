#include "main.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include "text.h"
#include "netTCP.h"
#include "netUDP.h"
#include "stateAndData.h"
#include "game.h"

struct gameCore{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    TTF_Font *pNamesFont;
    Text *pNames[MAXCLIENTS];
    int window_width, window_height;
    GameCoreData gData_local;

};


GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height){
    GameCore *pCore = malloc(sizeof(struct gameCore));
    pCore->pWindow = pWindow;
    pCore->pRenderer = pRenderer;
    pCore->window_height = width;
    pCore->window_height = height;
    pCore->pNamesFont = TTF_OpenFont(FONT_PATH_CORE_NAMES, FONT_SIZE_CORE_NAMES);
    SDL_memset(pCore->pNames, 0, sizeof(pCore->pNames));
    SDL_memset(&pCore->gData_local, 0, sizeof(GameCoreData));

    return pCore;
}


GameCoreData *getGData_local(GameCore *pCore){
    return &pCore->gData_local;
}

void destroyGameCore(GameCore *pCore){
    if(pCore->pNames) for(int i = 0; i < MAXCLIENTS; i++) destroyText(pCore->pNames);
    if(pCore->pNamesFont) TTF_CloseFont(pCore->pNamesFont);
    free(pCore);
}