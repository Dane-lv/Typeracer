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
    Text *pRoundText;
    Text *pInputText;
    SDL_Texture *pCars[MAXCLIENTS];
    int window_width, window_height;
    GameCoreData gData_local;
    TextData tData;
    SDL_FRect inputBox;
    char inputString[MAXINPSTR];
    int inpStrLen;

};


GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height){
    GameCore *pCore = malloc(sizeof(struct gameCore));
    pCore->pWindow = pWindow;
    pCore->pRenderer = pRenderer;
    pCore->window_width = width;
    pCore->window_height = height;
    pCore->pNamesFont = TTF_OpenFont(FONT_PATH_CORE_NAMES, FONT_SIZE_CORE_NAMES);
    pCore->pTextFont = TTF_OpenFont(FONT_PATH_GAME, FONT_SIZE_GAME);
    SDL_memset(pCore->pNames, 0, sizeof(pCore->pNames));
    SDL_memset(pCore->pCars, 0, sizeof(pCore->pCars));
    SDL_memset(&pCore->gData_local, 0, sizeof(GameCoreData));
    pCore->pInputText = NULL;
    SDL_memset(&pCore->tData, 0, sizeof(TextData));
    for(int i = 0; i < MAXCLIENTS; i++){
        if(i == 0) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car0.png");  SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 1) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car1.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 2) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car2.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 3) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car3.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
    }
    if(!readFromFile(pCore)) {printf("Error reading file %s: \n", SDL_GetError()); destroyGameCore(pCore); return NULL;}
    pCore->pRoundText = createRoundText(pCore->pRenderer, 255, 255, 255, pCore->pTextFont, pCore->tData.text, pCore->window_width/2 , pCore->window_height/1.5 - 50, pCore->window_width * 0.7);
    if(!pCore->pRoundText){printf("Error creating round text %s: \n", SDL_GetError()); destroyGameCore(pCore); return NULL;}
    pCore->inputBox.x = pCore->window_width/6.6 ;
    pCore->inputBox.y = pCore->window_height / 1.5 + 140;
    pCore->inputBox.w = pCore->window_width/1.5;
    pCore->inputBox.h = 50;
    SDL_memset(pCore->inputString, 0, sizeof(pCore->inputString));
    pCore->inpStrLen = 0;


    return pCore;
}

int gameCoreInputHandle(GameCore *pCore, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pCore->inpStrLen + strlen(event->text.text) < MAXINPSTR){
                strcat(pCore->inputString, event->text.text);
                pCore->inpStrLen = strlen(pCore->inputString);
                if(pCore->pInputText) destroyText(pCore->pInputText);
                pCore->pInputText = createText(pCore->pRenderer, 255, 255, 255, pCore->pTextFont, pCore->inputString, pCore->window_width/6.3,pCore->window_height / 1.5 + 162);
            }
            break;
          
    }
    return 0;
}



int readFromFile(GameCore *pCore){
    pCore->tData.chosenText = rand()% (NROFTEXTS)+1;
    printf("Text number %d was chosen\n", pCore->tData.chosenText);
    FILE *fp;
    fp = fopen("lib/resources/typeracertexts.txt", "r");
    if(fp == NULL){
        printf("Error opening text file %s: \n", SDL_GetError());
        return 0;
    }
    int fileTextNumber;
    char buffer[MAX_TEXT_LEN + 3]; 
    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        if(sscanf(buffer, "%d %[^\n]", &fileTextNumber, pCore->tData.text) == 2){
            if(fileTextNumber == pCore->tData.chosenText){
                printf("Loaded text %d\n", fileTextNumber);
                fclose(fp);
                return 1;
            }
        }
    }
    printf("Text number %d was not found\n", pCore->tData.chosenText);
    fclose(fp);
    return 0;
}

void renderInput(GameCore *pCore){
    if(pCore->pInputText) drawText(pCore->pInputText);
}

void renderRectangle(GameCore *pCore){
    SDL_SetRenderDrawColor(pCore->pRenderer, 255, 255, 255 ,255);
    SDL_RenderRect(pCore->pRenderer, &pCore->inputBox);
}

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
    renderInput(pCore);
    renderRectangle(pCore);
    drawText(pCore->pRoundText);
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
    if(pCore->pInputText) destroyText(pCore->pInputText);
    if(pCore->pRoundText) destroyText(pCore->pRoundText);
    if(pCore->pNamesFont) TTF_CloseFont(pCore->pNamesFont);
    if(pCore->pTextFont) TTF_CloseFont(pCore->pTextFont);
    for (int i = 0; i < MAXCLIENTS; i++){
        if (pCore->pCars[i])
            SDL_DestroyTexture(pCore->pCars[i]);
        if(pCore->pNames[i]){
            destroyText(pCore->pNames[i]);
        }
    }
    free(pCore);
}