#include "main.h"
#include <time.h>
#include <math.h>
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
    TTF_Font *pTextFont;
    Text *pNames[MAXCLIENTS];
    Text *pTexts[NROFTEXTS];
    SDL_Texture *pCars[MAXCLIENTS];
    int window_width, window_height;
    GameCoreData gData_local;
    TextData tData;

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
    SDL_memset(pCore->pTexts, 0, sizeof(pCore->pTexts));
    SDL_memset(&pCore->gData_local, 0, sizeof(GameCoreData));
    SDL_memset(&pCore->tData, 0, sizeof(TextData));
    for(int i = 0; i < MAXCLIENTS; i++){
        if(i == 0) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car0.png");  SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 1) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car1.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 2) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car2.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 3) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car3.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
    }
    
   
    srand(time(NULL));
    return pCore;
}

/*int readFromFile(GameCore *pCore){
    FILE *fp;
    fp = fopen("lib/resources/typeracertexts.txt", "r");
    if(fp != NULL){
        for(int i = 0; i < NROFTEXTS ; i++){
            fscanf(fp, " %[^*]", pCore->tData.texts[i]);
            fgetc(fp);
        }
        fclose(fp);
        pCore->tData.chosenText = rand()% (NROFTEXTS);

        return 1;
    }
    return 0;
}
*/
/*void renderText(GameCore *pCore){
    drawText(pCore->pTexts[pCore->tData.chosenText]);
}*/

void createNames(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        pCore->pNames[i] = createText(pCore->pRenderer, 233, 233, 233, pCore->pNamesFont, pCore->gData_local.players[i].playerName,300, 140 + i*85);
    }
}

void renderCars(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        SDL_FRect dst = {370, 110 + i * 85, 63, 63};
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