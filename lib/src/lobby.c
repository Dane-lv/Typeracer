#include "main.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include "text.h"
#include "lobby.h"
#include "stateAndData.h"



struct lobby{
    SDL_Renderer *pRenderer;
    SDL_Window *pWindow;
    int window_width, window_height;
    char playerName[MAXNAME];
    ClientData players_local[MAXCLIENTS]; // client gets info from the server here.
    int nameLength;
    TTF_Font *pFont;
    Text *pPromptText, *pInputText;  
    bool isTyping;
};


Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height){
    Lobby *pLobby = malloc(sizeof(struct lobby));
    pLobby->pRenderer = pRenderer;
    pLobby->pWindow = pWindow;
    pLobby->window_height = height;
    pLobby->window_width = width;
    pLobby->pFont = TTF_OpenFont(FONT_PATH_LOBBY, FONT_SIZE_LOBBY);
    if(!pLobby->pFont) {printf("Error lobby font init %s: \n", SDL_GetError()); return NULL;}
    pLobby->pPromptText = createText(pLobby->pRenderer, 244, 244 ,244, pLobby->pFont, "Enter name:", pLobby->window_width/2, pLobby->window_height/4);
    pLobby->pInputText = NULL;
    pLobby->playerName[0] = '\0';
    pLobby->nameLength = 0;
    pLobby->isTyping = true;
    memset(pLobby->players_local, 0, sizeof(pLobby->players_local));

    return pLobby;
}

int nameInputHandle(Lobby *pLobby, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pLobby->nameLength + strlen(event->text.text) < MAXNAME){
                strcat(pLobby->playerName, event->text.text);
                pLobby->nameLength = strlen(pLobby->playerName);
                if(pLobby->pInputText) destroyText(pLobby->pInputText);
                pLobby->pInputText = createText(pLobby->pRenderer, 222, 10, 222, pLobby->pFont, pLobby->playerName, pLobby->window_width/2, pLobby->window_height/4 + 80);
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pLobby->nameLength > 0){
                    pLobby->nameLength--;
                    pLobby->playerName[pLobby->nameLength] = '\0';
                    if(pLobby->pInputText) destroyText(pLobby->pInputText);
                    pLobby->pInputText = createText(pLobby->pRenderer, 122, 190, 222, pLobby->pFont, pLobby->playerName, pLobby->window_width/2, pLobby->window_height/4 + 100);
                }
            }
            else if(event->key.scancode == SDL_SCANCODE_RETURN || event->key.scancode == SDL_SCANCODE_KP_ENTER){
                if(pLobby->nameLength > 0) {return 1; pLobby->isTyping = false;}
                else {return -1;}
            }
            break;
        default: break;
    }
    return 0;
}

ClientData *lobby_getPlayersLocal(Lobby *pLobby){
    return pLobby->players_local;
}

char *getName(Lobby *pLobby){
    return pLobby->playerName;
}

bool isDoneTyping(Lobby *pLobby){
    return pLobby->isTyping;
}

void renderLobby(Lobby *pLobby){
    
    drawText(pLobby->pPromptText);
    if(pLobby->pInputText) drawText(pLobby->pInputText);
    if(isDoneTyping(pLobby)){
        
    }
    
}

void destroyLobby(Lobby *pLobby){
    if(pLobby->pInputText) destroyText(pLobby->pInputText); 
    if(pLobby->pFont) TTF_CloseFont(pLobby->pFont);
    if(pLobby->pPromptText) destroyText(pLobby->pPromptText);
    free(pLobby);
}