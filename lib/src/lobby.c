#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "lobby.h"
#include "stateAndData.h"
#include "text.h"
#include <stdlib.h>



struct lobby{

    SDL_Renderer *pRenderer;
    TTF_Font *pFont;
    Text *pPromptText, *pInputText;
    SDL_Window *pWindow;
    int window_width, window_height;
    char playerName[MAXNAME+1];
    bool isTyping;
    int nameLength;

};

Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height){
    Lobby *pLobby = malloc(sizeof(struct lobby));
    pLobby->pRenderer = pRenderer;
    pLobby->pWindow = pWindow;
    pLobby->playerName[0] = '\0';
    pLobby->isTyping = true;
    pLobby->window_height = height;
    pLobby->window_width = width;
    pLobby->pFont = TTF_OpenFont("lib/resources/arial.ttf", 65);
    if(!pLobby->pFont){
        printf("Error lobby font init %s: \n", SDL_GetError());
        destroyLobby(pLobby);
        return NULL;
    }
    pLobby->pPromptText = createText(pLobby->pRenderer, 155, 43, 11, pLobby->pFont, "Enter name:",pLobby->window_width/2 , pLobby->window_height/2 - 100);
    if(!pLobby->pPromptText){
        printf("Error prompttext create\n");
        destroyLobby(pLobby);
        return NULL;
    }
    pLobby->pInputText = NULL;
    pLobby->nameLength = 0;

    return pLobby;
}



int lobbyNameInputHandle(Lobby *pLobby, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pLobby->nameLength + strlen(event->text.text) < MAXNAME) {
                strcat(pLobby->playerName, event->text.text);
                pLobby->nameLength +=strlen(event->text.text);
                if(pLobby->pInputText) destroyText(pLobby->pInputText);
                pLobby->pInputText = createText(pLobby->pRenderer, 200, 198, 145, pLobby->pFont, pLobby->playerName, pLobby->window_width/2 , pLobby->window_height/2 - 40);
                return 0;
            }
            break;
        
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pLobby->nameLength > 0){
                    pLobby->playerName[--pLobby->nameLength] = '\0';
                    if(pLobby->pInputText) destroyText(pLobby->pInputText);
                    pLobby->pInputText = createText(pLobby->pRenderer, 200, 198, 145, pLobby->pFont, pLobby->playerName, pLobby->window_width/2 , pLobby->window_height/2 - 40);
                    return 0;
                }
            }
            else if(event->key.scancode == SDL_SCANCODE_RETURN || event->key.scancode == SDL_SCANCODE_KP_ENTER){
                if(pLobby->nameLength > 0){ //user pressed enter with non empty buffer
                    pLobby->isTyping = false;
                    return 1;
                }
                else{
                    return 0; //buffer is empty
                }
            }
        default: break;

    }
    return 0;
}

void renderNameInput(Lobby *pLobby){
    drawText(pLobby->pPromptText);
    if(pLobby->pInputText){
        drawText(pLobby->pInputText);
    }
}

bool isDoneTypingName(Lobby *pLobby){
    if(pLobby->isTyping) return false;
    return true;
}

char *returnName(Lobby *pLobby){
    return pLobby->playerName;
}

void destroyLobby(Lobby *pLobby){
    if(pLobby->pPromptText) destroyText(pLobby->pPromptText);
    if(pLobby->pInputText) destroyText(pLobby->pInputText);
    if(pLobby->pFont) TTF_CloseFont(pLobby->pFont);
    free(pLobby);
}



