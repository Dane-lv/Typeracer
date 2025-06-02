#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_net/SDL_net.h>
#include "text.h"
#include "stateAndData.h"

#define PORT 5665
#define MAX_CLIENTS 4   


struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    TTF_Font *pFont, *pClientsConnected;
    Text *pWaitingText, *pClientsConnectedText;
    GameState state;
    bool isRunning;
    NET_Server *pServer;
    NET_StreamSocket *pClients[MAX_CLIENTS];
    int nrOfClients;
};
typedef struct game Game;

bool init(Game *pGame);
void close(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame);
void messageBuffer(Game *pGame);
void manageMessage(Game *pGame, int clientIndex, const char *msg);
void updateGame(Game *pGame);
void acceptConnections(Game *pGame);
void updateClientsConnectedText(Game *pGame);

int main(){
    Game g = {0};

    if(!init(&g)) return 1;
    run(&g);
    close(&g);

    return 0;
}

bool init(Game *pGame){
    if (!SDL_Init(SDL_INIT_VIDEO)){
        printf("Error SDL Init: %s\n", SDL_GetError());

        return false;
    }
    if (!TTF_Init()){
        printf("Error: %s\n",SDL_GetError());
        SDL_Quit();
        return false;
    }
    if (!NET_Init()){
        printf("Error Net init: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    pGame->pServer = NET_CreateServer(NULL, PORT);
    if(!pGame->pServer){
        printf("Kunde inte skapa socket: %s\n", SDL_GetError());
        NET_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    pGame->nrOfClients = 0;
    memset(pGame->pClients, 0, sizeof(pGame->pClients));

    pGame->pWindow = SDL_CreateWindow("Skills Arena Server", 800, 600, SDL_WINDOW_RESIZABLE);
    if(!pGame->pWindow){
        printf("Error intializing window: %s\n", SDL_GetError());
        close(pGame);
        return false;
    }
    
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, NULL);
    if(!pGame->pRenderer){
        printf("Error initializing renderer: %s\n", SDL_GetError());
        close(pGame);
        return false;
    }

    pGame->pFont = TTF_OpenFont("lib/resources/arial.ttf", 50);
    if(!pGame->pFont){
        printf("Error font access: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->pClientsConnected = TTF_OpenFont("lib/resources/arial.ttf", 30);
    if(!pGame->pClientsConnected){
        printf("Error font access: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->pWaitingText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Waiting for clients", 800/2, 400/2);
    if(!pGame->pWaitingText){
        printf("Error waiting text: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->pClientsConnectedText = createText(pGame->pRenderer,140,24,65,pGame->pClientsConnected,"Clients connected: 0/4", 400, 300);
    if(!pGame->pClientsConnectedText){
        printf("Error clients connected text: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }

    pGame->isRunning = true;
    pGame->state = LOBBY;
    return true;
}


void handleInput(Game *pGame){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_EVENT_QUIT:
                pGame->isRunning = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if(event.key.scancode == SDL_SCANCODE_ESCAPE) {pGame->isRunning = false;} break;

            default: 
                break;    
            }    
    }
}

void renderGame(Game *pGame){
    switch(pGame->state){
        case LOBBY:
            drawText(pGame->pWaitingText);
            drawText(pGame->pClientsConnectedText);
            SDL_RenderPresent(pGame->pRenderer);
        default:
            break;
    }

}

void updateGame(Game *pGame) {
    switch (pGame->state) {
        case LOBBY: {
            acceptConnections(pGame);
            messageBuffer(pGame);
            break;
        }

        case ONGOING:

            break;

        case GAME_OVER:

            break;
    }
}

void acceptConnections(Game *pGame){
    if (pGame->nrOfClients < MAX_CLIENTS) { 
        NET_StreamSocket *pClient = NULL;
        while (NET_AcceptClient(pGame->pServer, &pClient)) {
            if (pClient == NULL) break;
            pGame->pClients[pGame->nrOfClients++] = pClient;
            updateClientsConnectedText(pGame);
            printf("Client %d has connected\n", pGame->nrOfClients);

            if (pGame->nrOfClients == MAX_CLIENTS) {
                printf("Starting the game\n");
                pGame->state = ONGOING;
                break;
            }
        }
    }
}

void messageBuffer(Game *pGame){
    for (int i = 0; i < pGame->nrOfClients; i++) {
        if (pGame->pClients[i]) {
            char buffer[BUFFERSIZE + 1] = {0};
            int bytesRead = NET_ReadFromStreamSocket(pGame->pClients[i], buffer, BUFFERSIZE);

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                manageMessage(pGame, i, buffer);
            }
            else if (bytesRead == -1){
                printf("Client %d disconnected or error: %s\n", i, SDL_GetError());
                NET_DestroyStreamSocket(pGame->pClients[i]);
                pGame->pClients[i] = NULL;
                pGame->nrOfClients--;
                updateClientsConnectedText(pGame);
            }
        }
    }
}

void manageMessage(Game *pGame, int clientIndex, const char *msg) {

}


void updateClientsConnectedText(Game *pGame){
    char buffer[64];
    sprintf(buffer, "Clients connected: %d", pGame->nrOfClients);
    if (pGame->pClientsConnectedText) {
        destroyText(pGame->pClientsConnectedText);
    }
    pGame->pClientsConnectedText = createText(pGame->pRenderer, 140, 24, 65,
                                            pGame->pClientsConnected, buffer, 
                                            600 / 2, 300 / 2);
}





void run(Game *pGame){ 

    while(pGame->isRunning){
        handleInput(pGame);
        updateGame(pGame);
        renderGame(pGame);
      
    }

}

void close(Game *pGame){
    for(int i = 0; i < pGame->nrOfClients; ++i){
        if(pGame->pClients[i]){
            NET_DestroyStreamSocket(pGame->pClients[i]);
        }
    }
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWaitingText) destroyText(pGame->pWaitingText);
    if(pGame->pClientsConnectedText) destroyText(pGame->pClientsConnectedText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    
    TTF_Quit(); 
    NET_Quit();
    SDL_Quit();
}