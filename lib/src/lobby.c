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
    Text *pReadyText[MAXPLAYERS];  // Added missing array for ready status text
    Text *pHostText;  // Text to show who is host
    Text *pInstructionText;  // Instructions for host
    SDL_Window *pWindow;
    int window_width, window_height;
    char playerName[MAXNAME+1];
    bool isTyping;
    int nameLength;
    bool isHost;  // Track if this client is the host

    char names[MAXPLAYERS][MAXNAME]; // Filled by server to the clients
    bool ready[MAXPLAYERS];
    int nrOfPlayers;

};

Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height, bool isHost){
    Lobby *pLobby = malloc(sizeof(struct lobby));
    pLobby->pRenderer = pRenderer;
    pLobby->pWindow = pWindow;
    pLobby->playerName[0] = '\0';
    pLobby->isTyping = true;
    pLobby->isHost = isHost;
    pLobby->window_height = height;
    pLobby->window_width = width;
    pLobby->pFont = TTF_OpenFont("lib/resources/arial.ttf", 24);
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
    pLobby->pHostText = NULL;
    pLobby->pInstructionText = NULL;
    pLobby->nameLength = 0;
    pLobby->nrOfPlayers = 0;
    for(int i = 0; i < MAXPLAYERS; i++){
        pLobby->ready[i] = false;
        pLobby->names[i][0] = '\0';
        pLobby->pNameText[i] = NULL;
        pLobby->pReadyText[i] = NULL;  // Initialize ready text pointers
    }

    return pLobby;
}

void lobbySetReady(Lobby *pLobby, int idx, bool ready){
    if(idx < 0 || idx >= pLobby->nrOfPlayers) return;

    pLobby->ready[idx] = ready;

    if (pLobby->pReadyText[idx]) destroyText(pLobby->pReadyText[idx]);

    if (ready) {
        pLobby->pReadyText[idx] = createText(pLobby->pRenderer, 
            0, 255, 0, pLobby->pFont, "Ready",
            LIST_X + 300, LIST_Y_START + idx * LIST_Y_STEP);
    } else {
        pLobby->pReadyText[idx] = createText(pLobby->pRenderer, 
            255, 0, 0, pLobby->pFont, "Not Ready",
            LIST_X + 300, LIST_Y_START + idx * LIST_Y_STEP);
    }
}

bool lobbyAllPlayersReady(Lobby *pLobby){
    if (pLobby->nrOfPlayers == 0) return false;

    for (int i = 0; i < pLobby->nrOfPlayers; i++){
        if (!pLobby->ready[i]) return false;
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

void lobbyAddPlayer(Lobby *pLobby, char *name)
{
    if (!name || name[0] == '\0') return;

    // Find a free slot
    int idx = -1;
    for (int i = 0; i < pLobby->nrOfPlayers; ++i) {
        if (pLobby->names[i][0] == '\0') {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        if (pLobby->nrOfPlayers >= MAXPLAYERS)
            return;
        idx = pLobby->nrOfPlayers++;
    }
    strncpy(pLobby->names[idx], name, MAXNAME-1);
    pLobby->names[idx][MAXNAME-1] = '\0';

    if (pLobby->pNameText[idx])
        destroyText(pLobby->pNameText[idx]);

    // Server already adds (HOST) indication, so just display the name as received
    pLobby->pNameText[idx] = createText(pLobby->pRenderer,255, 255, 255,pLobby->pFont,pLobby->names[idx],LIST_X,LIST_Y_START + idx * LIST_Y_STEP);
    
    // Add initial "NOT READY" status
    pLobby->ready[idx] = false;
    if (pLobby->pReadyText[idx]) destroyText(pLobby->pReadyText[idx]);
    pLobby->pReadyText[idx] = createText(pLobby->pRenderer, 
        255, 0, 0, pLobby->pFont, "Not Ready",
        LIST_X + 300, LIST_Y_START + idx * LIST_Y_STEP);

    // Add instruction text for host (only show after first player joins)
    if (pLobby->isHost && !pLobby->pInstructionText) {
        pLobby->pInstructionText = createText(pLobby->pRenderer, 255, 255, 0, pLobby->pFont, 
            "Press space when all players ready to start",
            pLobby->window_width/2, pLobby->window_height - 100);
    }
}

bool lobbyIsPlayerReady(Lobby *pLobby, char *playerName) {
    for (int i = 0; i < pLobby->nrOfPlayers; i++) {
        if (strcmp(pLobby->names[i], playerName) == 0) {
            return pLobby->ready[i];
        }
    }
    return false;
}

void lobbySetPlayerReady(Lobby *pLobby, char *playerName, bool ready) {
    for (int i = 0; i < pLobby->nrOfPlayers; i++) {
        if (strcmp(pLobby->names[i], playerName) == 0) {
            pLobby->ready[i] = ready;
            
            if (pLobby->pReadyText[i]) destroyText(pLobby->pReadyText[i]);

            if (ready) {
                pLobby->pReadyText[i] = createText(pLobby->pRenderer, 
                    0, 255, 0, pLobby->pFont, "Ready",
                    LIST_X + 300, LIST_Y_START + i * LIST_Y_STEP);
            } else {
                pLobby->pReadyText[i] = createText(pLobby->pRenderer, 
                    255, 0, 0, pLobby->pFont, "Not Ready",
                    LIST_X + 300, LIST_Y_START + i * LIST_Y_STEP);
            }
            return;
        }
    }
}

void renderLobby(Lobby *pLobby){
    for(int i = 0; i < pLobby->nrOfPlayers; i++){
        if(pLobby->pNameText[i]) drawText(pLobby->pNameText[i]);
        if(pLobby->pReadyText[i]) drawText(pLobby->pReadyText[i]);
    }
    if(pLobby->pInstructionText) drawText(pLobby->pInstructionText);
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
    if(pLobby->pHostText) destroyText(pLobby->pHostText);
    if(pLobby->pInstructionText) destroyText(pLobby->pInstructionText);
    for(int i = 0; i < MAXPLAYERS; i++){
        if(pLobby->pNameText[i]) destroyText(pLobby->pNameText[i]);
        if(pLobby->pReadyText[i]) destroyText(pLobby->pReadyText[i]);
    }
    if(pLobby->pFont) TTF_CloseFont(pLobby->pFont);
    free(pLobby);
}

bool lobbyGetReady(Lobby *pLobby, int idx) {
    if(idx < 0 || idx >= pLobby->nrOfPlayers) return false;
    return pLobby->ready[idx];
}



