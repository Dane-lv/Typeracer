#include "main.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include "text.h"
#include "lobby.h"
#include "netTCP.h"
#include "stateAndData.h"



struct lobby{
    SDL_Renderer *pRenderer;
    SDL_Window *pWindow;
    int window_width, window_height;
    char playerName[MAXNAME];
    LobbyData lobby_local; // client gets info from the server here.
    int nameLength;
    TTF_Font *pFont;
    Text *pPromptText, *pInputText, *pHostText, *pWaitingHostStart, *pPressSpaceText, *pHost_playersReadyText;  
    Text *lobbyNames[MAXCLIENTS];
    Text *lobbyPlayerStatus[MAXCLIENTS];
    bool lobbyChanged;
    bool isTyping;
    bool isReady;
    bool isHost;
 
};


Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height, bool hostCheck){
    Lobby *pLobby = malloc(sizeof(struct lobby));
    pLobby->pRenderer = pRenderer;
    pLobby->pWindow = pWindow;
    pLobby->window_height = height;
    pLobby->window_width = width;
    pLobby->pFont = TTF_OpenFont(FONT_PATH_LOBBY, FONT_SIZE_LOBBY);
    if(!pLobby->pFont) {printf("Error lobby font init %s: \n", SDL_GetError()); return NULL;}
    pLobby->pPromptText = createText(pLobby->pRenderer, 255, 225, 225, pLobby->pFont, "Enter name:", pLobby->window_width/2, pLobby->window_height/4, true);
    pLobby->pInputText = NULL;
    pLobby->playerName[0] = '\0';
    pLobby->nameLength = 0;
    pLobby->isTyping = true;
    memset(&pLobby->lobby_local, 0, sizeof(LobbyData));
    memset(pLobby->lobbyNames, 0, sizeof(pLobby->lobbyNames));
    memset(pLobby->lobbyPlayerStatus, 0, sizeof(pLobby->lobbyPlayerStatus));
    pLobby->lobbyChanged = false;
    pLobby->isReady = false;

    pLobby->pHostText = NULL;
    pLobby->pPressSpaceText = NULL;
    pLobby->pWaitingHostStart = NULL;
    pLobby->pHost_playersReadyText = NULL;
    pLobby->isHost = hostCheck;




    return pLobby;
}

int nameInputHandle(Lobby *pLobby, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pLobby->nameLength + strlen(event->text.text) < MAXNAME){
                strcat(pLobby->playerName, event->text.text);
                pLobby->nameLength = strlen(pLobby->playerName);
                if(pLobby->pInputText) destroyText(pLobby->pInputText);
                pLobby->pInputText = createText(pLobby->pRenderer, 255, 255, 255, pLobby->pFont, pLobby->playerName, pLobby->window_width/2, pLobby->window_height/4 + 80, true);
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pLobby->nameLength > 0){
                    pLobby->nameLength--;
                    pLobby->playerName[pLobby->nameLength] = '\0';
                    if(pLobby->pInputText) destroyText(pLobby->pInputText);
                    pLobby->pInputText = createText(pLobby->pRenderer,  255, 255, 255, pLobby->pFont, pLobby->playerName, pLobby->window_width/2, pLobby->window_height/4 + 80, true);
                }
            }
            else if(event->key.scancode == SDL_SCANCODE_RETURN || event->key.scancode == SDL_SCANCODE_KP_ENTER){
                if(pLobby->nameLength > 0) {pLobby->isTyping = false; return 1; } //is typing -> false
                else {return -1;}
            }
            break;
        default: break;
    }
    return 0;
}

int lobbyEventHandle(Lobby *pLobby, SDL_Event *event){
    switch (event->type){
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_SPACE){ // SPACE FOR "PLAYER READY"
               if(pLobby->isHost == true && arePlayersReady(pLobby) == true){
                    printf("All players ready, host starting game...\n");
                    return 2;
               }
               else{
                    if(pLobby->isReady == false) pLobby->isReady = true; 
                    if(pLobby->isReady == true) pLobby->isReady = false; 
    
                    return 1;
               }
            }
        default: break;
    }
    return 0;
}

bool arePlayersReady(Lobby *pLobby){
    int ready_counter = 0;
    for(int i = 0; i < pLobby->lobby_local.nrOfPlayers; i++){
        if(pLobby->lobby_local.players[i].isReady == true){
            ready_counter++;
        }
    }
    if(ready_counter == pLobby->lobby_local.nrOfPlayers){
        return true;
    }
    return false;
}

bool getReadyStatus(Lobby *pLobby){
    return pLobby->isReady;
}

LobbyData *getLobbyLocal(Lobby *pLobby){
    return &pLobby->lobby_local;
}

char *getName(Lobby *pLobby){
    return pLobby->playerName;
}

bool isStillTyping(Lobby *pLobby){
    return pLobby->isTyping;
}

void renderLobby(Lobby *pLobby){
    if(isStillTyping(pLobby)){
        drawText(pLobby->pPromptText);
        if(pLobby->pInputText) drawText(pLobby->pInputText);
    }
    else renderNamesAndStatus(pLobby);
}

void setLobbyChanged(Lobby *pLobby, bool changed){
    pLobby->lobbyChanged = changed;
}

void updateLobby(Lobby *pLobby){
    if(pLobby == NULL) return;
    if(pLobby->lobbyChanged){
        if(pLobby->pHostText){ destroyText(pLobby->pHostText); pLobby->pHostText = NULL;}
        if(pLobby->pWaitingHostStart) destroyText(pLobby->pWaitingHostStart); pLobby->pWaitingHostStart = NULL;
        if(pLobby->pPressSpaceText) destroyText(pLobby->pPressSpaceText);  pLobby->pPressSpaceText = NULL;
        if(pLobby->pHost_playersReadyText) destroyText(pLobby->pHost_playersReadyText);  pLobby->pHost_playersReadyText = NULL;
        for(int i = 0; i < MAXCLIENTS; i++){
            if(pLobby->lobbyNames[i]) {
                destroyText(pLobby->lobbyNames[i]);
                pLobby->lobbyNames[i] = NULL;
            }
            if(pLobby->lobbyPlayerStatus[i]) {
                destroyText(pLobby->lobbyPlayerStatus[i]);
                pLobby->lobbyPlayerStatus[i] = NULL;
            }

        }
        for(int i = 0; i < pLobby->lobby_local.nrOfPlayers; i++){
            pLobby->lobbyNames[i] = createText(pLobby->pRenderer, 247, 225, 225, pLobby->pFont,
                                              pLobby->lobby_local.players[i].playerName,
                                              pLobby->window_width/6+40, 150 + i*100, true);

            if(pLobby->lobby_local.players[i].isHost == true){
                pLobby->pHostText = createText(pLobby->pRenderer, 234, 200 ,0, pLobby->pFont,
                                               "HOST", 600, 150, true);
            }
            if(pLobby->lobby_local.players[i].isReady == false){
                pLobby->lobbyPlayerStatus[i] = createText(pLobby->pRenderer,255,0,0, pLobby->pFont, "NOT READY",
                                                        880, 150 + i*100, true);
                pLobby->pPressSpaceText = createText(pLobby->pRenderer, 230, 230 ,230, pLobby->pFont, "press space if ready",
                                                        pLobby->window_width/2, 650, true);
            }
            else if(pLobby->lobby_local.players[i].isReady == true){
                pLobby->lobbyPlayerStatus[i] = createText(pLobby->pRenderer,0,128,0, pLobby->pFont, "READY",
                                                        880, 150 + i*100, true);
            }
            if(arePlayersReady(pLobby) && pLobby->isHost == false){
                pLobby->pWaitingHostStart = createText(pLobby->pRenderer, 230, 230 ,230, pLobby->pFont, "waiting for host to start",
                                                         pLobby->window_width/2, 650, true);
            }
            if(arePlayersReady(pLobby) && pLobby->isHost == true){
                pLobby->pHost_playersReadyText = createText(pLobby->pRenderer, 234, 200 ,0, pLobby->pFont, "press space to start game",
                                                         pLobby->window_width/2, 650, true);
            }   

        }
        pLobby->lobbyChanged = false;
    }
}

void renderNamesAndStatus(Lobby *pLobby){

    if(pLobby->pHostText) drawText(pLobby->pHostText);
    if(pLobby->pWaitingHostStart) drawText(pLobby->pWaitingHostStart);
    if(pLobby->pPressSpaceText) drawText(pLobby->pPressSpaceText);
    if(pLobby->pHost_playersReadyText) drawText(pLobby->pHost_playersReadyText);
       
    
    for(int i = 0; i < pLobby->lobby_local.nrOfPlayers; i++){
        if(pLobby->lobbyNames[i]) drawText(pLobby->lobbyNames[i]);
        if(pLobby->lobbyPlayerStatus[i]) drawText(pLobby->lobbyPlayerStatus[i]);
      
    }
}

bool hostCheck(Lobby *pLobby){
    return pLobby->isHost;
}


void destroyLobby(Lobby *pLobby){
    
    if(pLobby->pWaitingHostStart) destroyText(pLobby->pWaitingHostStart); pLobby->pWaitingHostStart = NULL;
    if(pLobby->pPressSpaceText) destroyText(pLobby->pPressSpaceText);  pLobby->pPressSpaceText = NULL;
    if(pLobby->pHost_playersReadyText) destroyText(pLobby->pHost_playersReadyText);  pLobby->pHost_playersReadyText = NULL;
    if(pLobby->pHostText) destroyText(pLobby->pHostText);
    for(int i = 0; i < MAXCLIENTS; i++){
        if(pLobby->lobbyNames[i]) destroyText(pLobby->lobbyNames[i]);
        if(pLobby->lobbyPlayerStatus[i]) destroyText(pLobby->lobbyPlayerStatus[i]);
    }
    if(pLobby->pInputText) destroyText(pLobby->pInputText); 
    if(pLobby->pFont) TTF_CloseFont(pLobby->pFont);
    if(pLobby->pPromptText) destroyText(pLobby->pPromptText);
    free(pLobby);
}