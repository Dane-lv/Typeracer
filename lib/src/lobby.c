#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "lobby.h"
#include "stateAndData.h"
#include "text.h"
#include <stdlib.h>

#define LIST_X 100 // Name render placement
#define LIST_Y_START 100
#define LIST_Y_STEP 75



struct lobby{

    SDL_Renderer *pRenderer;
    TTF_Font *pFont;
    Text *pPromptText, *pInputText, *pNameText[MAXPLAYERS];
    SDL_Window *pWindow;
    int window_width, window_height;
    char playerName[MAXNAME+1];
    bool isTyping;
    int nameLength;

    char names[MAXPLAYERS][MAXNAME]; // Filled by server to the clients
    bool ready[MAXPLAYERS];
    int nrOfPlayers;

};

Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height){
    Lobby *pLobby = malloc(sizeof(struct lobby));
    pLobby->pRenderer = pRenderer;
    pLobby->pWindow = pWindow;
    pLobby->playerName[0] = '\0';
    pLobby->isTyping = true;
    pLobby->window_height = height;
    pLobby->window_width = width;
    pLobby->pFont = TTF_OpenFont("lib/resources/arial.ttf", 50);
    if(!pLobby->pFont){
        printf("Error lobby font init %s: \n", SDL_GetError());
        destroyLobby(pLobby);
        return NULL;
    }
    pLobby->pPromptText = createText(pLobby->pRenderer, 155, 43, 11, pLobby->pFont, "Enter name:",pLobby->window_width/2 , pLobby->window_height/2 - 130);
    if(!pLobby->pPromptText){
        printf("Error prompttext create\n");
        destroyLobby(pLobby);
        return NULL;
    }
    pLobby->pInputText = NULL;
    pLobby->nameLength = 0;
    pLobby->nrOfPlayers = 0;
    for(int i = 0; i < MAXPLAYERS; i++){

        pLobby->ready[i] = false;
        pLobby->names[i][0] = '\0';
        pLobby->pNameText[i] = NULL;
    }

    return pLobby;
}

void lobbySetReady(Lobby *l, int idx, bool ready){
    if(idx < 0 || idx >= l->nrOfPlayers) return;

    l->ready[idx] = ready;

    if (l->pReadyText[idx]) destroyText(l->pReadyText[idx]);

    if (ready) {
        l->pReadyText[idx] = createText(l->pRenderer, 
            0, 255, 0, l->pFont, "Ready",
            LIST_X + 300, LIST_Y_START + idx * LIST_Y_STEP);
    } else {
        l->pReadyText[idx] = createText(l->pRenderer, 
            255, 0, 0, l->pFont, "Not Ready",
            LIST_X + 300, LIST_Y_START + idx * LIST_Y_STEP);
    }
}

bool lobbyAllPlayersReady(Lobby *l){
    if (l->nrOfPlayers == 0) return false;

    for (int i = 0; i < l->nrOfPlayers; i++){
        if (!l->ready[i]) return false;
    }

    return true;
}



int lobbyNameInputHandle(Lobby *pLobby, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pLobby->nameLength + strlen(event->text.text) < MAXNAME) { //register input and render create text
                strcat(pLobby->playerName, event->text.text);
                pLobby->nameLength +=strlen(event->text.text);
                if(pLobby->pInputText) destroyText(pLobby->pInputText);
                pLobby->pInputText = createText(pLobby->pRenderer, 233, 233, 233, pLobby->pFont, pLobby->playerName, pLobby->window_width/2 , pLobby->window_height/2 - 40);
                return 0;
            }
            break;
        
        case SDL_EVENT_KEY_DOWN: //destroy the last letter after backspace and create a new text
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pLobby->nameLength > 0){
                    pLobby->playerName[--pLobby->nameLength] = '\0';
                    if(pLobby->pInputText) destroyText(pLobby->pInputText);
                    pLobby->pInputText = createText(pLobby->pRenderer, 233, 233, 233, pLobby->pFont, pLobby->playerName, pLobby->window_width/2 , pLobby->window_height/2 - 40);
                    return 0;
                }
            }
            else if(event->key.scancode == SDL_SCANCODE_RETURN || event->key.scancode == SDL_SCANCODE_KP_ENTER){
                if(pLobby->nameLength > 0){ // user pressed enter with non empty buffer
                    pLobby->isTyping = false; // signal to the main 
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
    drawText(pLobby->pPromptText); //render prompt while entering name
    if(pLobby->pInputText){
        drawText(pLobby->pInputText);
    }
}

void lobbyAddPlayer(Lobby *l, char *name)
{
    if (!name || name[0] == '\0') return;

    // Find a free slot
    int idx = -1;
    for (int i = 0; i < l->nrOfPlayers; ++i) {
        if (l->names[i][0] == '\0') {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        if (l->nrOfPlayers >= MAXPLAYERS)
            return;
        idx = l->nrOfPlayers++;
    }
    strncpy(l->names[idx], name, MAXNAME-1);
    l->names[idx][MAXNAME-1] = '\0';

    if (l->pNameText[idx])
        destroyText(l->pNameText[idx]);

    l->pNameText[idx] = createText(l->pRenderer,255, 255, 255,l->pFont,l->names[idx],LIST_X,LIST_Y_START + idx * LIST_Y_STEP);
}



void renderLobby(Lobby *pLobby){
    for(int i = 0; i < pLobby->nrOfPlayers; i++){
        if(pLobby->pNameText[i]) drawText(pLobby->pNameText[i]);
    }
}

bool isDoneTypingName(Lobby *pLobby){ // signal to the main
    if(pLobby->isTyping) return false;
    return true;
}

char *returnName(Lobby *pLobby){
    return pLobby->playerName;
}

void destroyLobby(Lobby *pLobby){
    if(pLobby->pPromptText) destroyText(pLobby->pPromptText);
    if(pLobby->pInputText) destroyText(pLobby->pInputText);
    for(int i = 0; i < MAXPLAYERS; i++){
        if(pLobby->pNameText[i]) destroyText(pLobby->pNameText[i]);
    }
    if(pLobby->pFont) TTF_CloseFont(pLobby->pFont);
    free(pLobby);
}



