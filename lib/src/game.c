#include "main.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
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
    SDL_Texture *pCars[MAXCLIENTS];
    int window_width, window_height;
    GameCoreData gData_local;

};


GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height){
    GameCore *pCore = malloc(sizeof(struct gameCore));
    pCore->pWindow = pWindow;
    pCore->pRenderer = pRenderer;
    pCore->window_width = width;
    pCore->window_height = height;
    pCore->pNamesFont = TTF_OpenFont(FONT_PATH_CORE_NAMES, FONT_SIZE_CORE_NAMES);
    SDL_memset(pCore->pNames, 0, sizeof(pCore->pNames));
    SDL_memset(pCore->pCars, 0, sizeof(pCore->pCars));
    SDL_memset(&pCore->gData_local, 0, sizeof(GameCoreData));
    for(int i = 0; i < MAXCLIENTS; i++){
        if(i == 0) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car0.png");  SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 1) pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car1.png");
        if(i == 2) pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car2.png");
        if(i == 3) pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car3.png");
    }

    return pCore;
}

void createNames(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        pCore->pNames[i] = createText(pCore->pRenderer, 233, 233, 233, pCore->pNamesFont, pCore->gData_local.players[i].playerName, 65, 65 + i*65);
    }
}

void renderCars(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        SDL_FRect dst = {145, 35 + i * 65, 65, 65};
        if(pCore->pCars[i]) SDL_RenderTexture(pCore->pRenderer, pCore->pCars[i], NULL, &dst);
    }
}


void renderCore(GameCore *pCore){
    renderNames(pCore);
    renderCars(pCore);
}

void renderNames(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        drawText(pCore->pNames[i]);
    }
}


GameCoreData *getGData_local(GameCore *pCore){
    return &pCore->gData_local;
}

void destroyGameCore(GameCore *pCore){
    for (int i = 0; i < MAXCLIENTS; i++){
        if (pCore->pCars[i])
            SDL_DestroyTexture(pCore->pCars[i]);
    }
    free(pCore);
}