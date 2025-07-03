#include "main.h"
#include <time.h>
#include <math.h>
#include <string.h>
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
    Text *pTextAsWords[MAXTEXTWORD];
    Text *pInputText;
    SDL_Texture *pCars[MAXCLIENTS];
    int window_width, window_height;
    GameCoreData gData_local;
    TextData tData;
    SDL_FRect inputBox;
    SDL_FRect blinkingCursor;
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
    SDL_memset(pCore->pTextAsWords, 0, sizeof(pCore->pTextAsWords));
    pCore->pInputText = NULL;
    SDL_memset(&pCore->tData, 0, sizeof(TextData));
    for(int i = 0; i < MAXCLIENTS; i++){
        if(i == 0) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car0.png");  SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 1) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car1.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 2) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car2.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 3) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car3.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
    }
    if(!readFromFile(pCore)) {printf("Error reading file %s: \n", SDL_GetError()); destroyGameCore(pCore); return NULL;}
    pCore->inputBox.x = pCore->window_width/6.6 ;
    pCore->inputBox.y = pCore->window_height / 1.5 + 140;
    pCore->inputBox.w = pCore->window_width/1.5;
    pCore->inputBox.h = 50;
    pCore->blinkingCursor.x = pCore->window_width/6.3-2;
    pCore->blinkingCursor.y = pCore->window_height/1.5 - 150;
    pCore->blinkingCursor.w = 2;
    pCore->blinkingCursor.h = 38;
    SDL_memset(pCore->inputString, 0, sizeof(pCore->inputString));
    pCore->inpStrLen = 0;
    parseText(pCore);
    createTextAsWords(pCore);
    pCore->tData.currentWordIndex = 0;


    return pCore;
}

void parseText(GameCore *pCore){ // split text into words
    pCore->tData.nrOfWords = 0;
    char *token = strtok(pCore->tData.text, " ");
    while(token && pCore->tData.nrOfWords < 100){
        pCore->tData.words[pCore->tData.nrOfWords++] = token;
        token = strtok(NULL, " ");
    }
}

void createTextAsWords(GameCore *pCore){
    int startX = pCore->window_width/6.3;
    int x = startX;
    int y = pCore->window_height/1.5 - 150;
    int lineHeight, spaceW;
    TTF_GetStringSize(pCore->pTextFont, " ", 0, &spaceW, &lineHeight); // calculate size of space
    for(int i = 0; i < pCore->tData.nrOfWords; i++){
        char *word = pCore->tData.words[i];
        if(word == NULL) continue;
        int wordW, wordH;         // Calculate how wide this word will be
                                    // Check if word would exceed right boundary
                                    // Right boundary = leftMargin + 70% of screen width
        TTF_GetStringSize(pCore->pTextFont, word, 0, &wordW, &wordH);
        if(x + wordW > (pCore->window_width/6.3 + pCore->window_width * 0.7)){
            x = startX;
            y += lineHeight;
        }
        pCore->pTextAsWords[i] = createText(pCore->pRenderer,255,255,255, pCore->pTextFont, word, x, y);
        SDL_FRect *rect = getRect(pCore->pTextAsWords[i]);
        rect->x = x;
        rect->y = y;
        x += wordW + spaceW;
    }
}

/*void checkInput(GameCore *pCore){
    for(int i = 0; i < pCore->tData.nrOfWords; i++){
        if(strcmp(pCore->inputString, pCore->tData.words[i]) == 0){
            printf("Words match!\n");
        }
    }
}*/


int gameCoreInputHandle(GameCore *pCore, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pCore->inpStrLen + strlen(event->text.text) < MAXINPSTR){
                strcat(pCore->inputString, event->text.text);
                pCore->inpStrLen = strlen(pCore->inputString);
                if(pCore->pInputText) destroyText(pCore->pInputText);
                pCore->pInputText = createText(pCore->pRenderer, 255, 255, 255, pCore->pTextFont, pCore->inputString, pCore->window_width/6.3,pCore->window_height / 1.5 + 165);
                pCore->blinkingCursor.x = pCore->blinkingCursor.x + 40;
            }
            if(event->text.text[0] == ' '){
                pCore->inputString[pCore->inpStrLen-1] = '\0';
                if(strcmp(pCore->inputString, pCore->tData.words[pCore->tData.currentWordIndex]) == 0){
                    pCore->tData.currentWordIndex++;
                }
                SDL_memset(pCore->inputString, 0, sizeof(pCore->inputString));
                pCore->inpStrLen = 0;
                if(pCore->pInputText) destroyText(pCore->pInputText);
                pCore->pInputText = NULL;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pCore->inpStrLen > 0){
                    pCore->inpStrLen--;
                    pCore->inputString[pCore->inpStrLen] = '\0';
                    if(pCore->pInputText) destroyText(pCore->pInputText);
                    pCore->pInputText = createText(pCore->pRenderer, 255, 255, 255, pCore->pTextFont, pCore->inputString, pCore->window_width/6.3,pCore->window_height / 1.5 + 165);
                }
            }
          
    }
    return 0;
}

void renderBlinkingCursor(GameCore *pCore){
    SDL_SetRenderDrawColor(pCore->pRenderer,220,254,250,230);
    SDL_RenderFillRect(pCore->pRenderer, &pCore->blinkingCursor);
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

void renderText(GameCore *pCore){
    for(int i = 0; i < pCore->tData.nrOfWords; i++){
        if(pCore->pTextAsWords[i]){
            drawText(pCore->pTextAsWords[i]);
        }
    }    
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
    renderBlinkingCursor(pCore);
    renderText(pCore);
    renderInput(pCore);
    renderRectangle(pCore);
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
    for(int i = 0; i < MAXTEXTWORD; i++){
        if(pCore->pTextAsWords[i]) destroyText(pCore->pTextAsWords[i]);
    }
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