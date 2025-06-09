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
#include "menu.h"
#include "network.h"
#include "lobby.h"

#define PORT 7777


struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    bool isRunning;
    GameState state;
    ClientNetwork *pClientNet;
    ServerNetwork *pServerNet;
    Menu *pMenu;
    IpBar *pIpBar;
    Lobby *pLobby;

};
typedef struct game Game;

bool init(Game *pGame);
void close(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame);
void updateGame(Game *pGame);

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

    pGame->pWindow = SDL_CreateWindow("Typeracer", 800, 600, SDL_WINDOW_RESIZABLE);
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
    pGame->pMenu = createMenu(pGame->pRenderer, pGame->pWindow, 800, 600);
    if(!pGame->pMenu){
        printf("Error menu init: %s\n", SDL_GetError());
        close(pGame);
        return false;
    }

    pGame->pClientNet = NULL;
    pGame->pServerNet = NULL;
    pGame->pIpBar = NULL;
    pGame->pLobby = NULL;
    pGame->state = MENU;
    pGame->isRunning = true;
    return true;
}


void handleInput(Game *pGame){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_EVENT_QUIT){
            pGame->isRunning = false;
            return;
        }

        int menuOptionClicked, ipBarResult, lobbyNameResult;

        switch(pGame->state){
            case MENU:
                menuOptionClicked = menuOptionsEvent(pGame->pMenu, &event);
                if(menuOptionClicked == 1){ //clicked "connect"
                    pGame->pIpBar = createIpBar(pGame->pRenderer, pGame->pWindow, 800, 600);
                    if(!pGame->pIpBar){
                        destroyIpBar(pGame->pIpBar);
                        printf("Error ipbar init no memory: %s\n", SDL_GetError());
                        return;
                    }
                    else {
                        pGame->state = ENTER_IP;
                        SDL_StartTextInput(pGame->pWindow);
                        break; 
                    }
                }
                if(menuOptionClicked == 2){ //clicked "host game"
                    pGame->pServerNet = createServerNetwork(PORT);
                    if(!pGame->pServerNet){
                        printf("Error server network init %s\n", SDL_GetError());
                        return;
                    }
                    pGame->pClientNet = createClientNetwork("127.0.0.1", PORT);
                    if(!pGame->pClientNet){
                        printf("Error client network init (host): %s\n", SDL_GetError());
                        destroyServerNetwork(pGame->pServerNet);
                        return;
                    }
                    if(!connectToServer(pGame->pClientNet)){
                        printf("Host failed to connect to own server: %s\n", SDL_GetError());
                        destroyClientNetwork(pGame->pClientNet);
                        destroyServerNetwork(pGame->pServerNet);
                        return;
                    }
                    int result = holdUntilConnected(pGame->pClientNet, 500);
                    if (result != 1) {
                        printf("Host failed to complete connection to own server: %s\n", SDL_GetError());
                        destroyClientNetwork(pGame->pClientNet);
                        destroyServerNetwork(pGame->pServerNet);
                        return;
                    }
                    pGame->pLobby = createLobby(pGame->pRenderer, pGame->pWindow, 800, 600);
                    if(!pGame->pLobby){
                        printf("Error lobby init %s\n", SDL_GetError());
                        return;
                    }

                    pGame->state = LOBBY;
                    break;
                }
                
                if(menuOptionClicked == 3){
                    // settingswindow
                }
                break;
                
            case ENTER_IP:
                ipBarResult = IpBarHandle(pGame->pIpBar, &event);
                if(ipBarResult == 1){ //user pressed enter with nonempty buffer
                    char *ipString = getIpAdress(pGame->pIpBar);
                    pGame->pClientNet = createClientNetwork(ipString, PORT);
                    if(!pGame->pClientNet){
                       printf("Error client network init %s\n", SDL_GetError());
                       return;
                    }
                    else{
                         if(!connectToServer(pGame->pClientNet)){
                            destroyClientNetwork(pGame->pClientNet);
                            showIpBarStatus(pGame->pIpBar, "Failed to connect", 255, 0, 0);
                         }
                         else{
                            SDL_StopTextInput(pGame->pWindow);
                            if(pGame->pIpBar){
                                destroyIpBar(pGame->pIpBar);
                                pGame->pIpBar = NULL;
                            }
                            pGame->pLobby = createLobby(pGame->pRenderer, pGame->pWindow, 800, 600);
                            if(!pGame->pLobby){
                                printf("Error lobby init %s: \n", SDL_GetError());
                                return;
                            }
                            pGame->state = LOBBY;
                         }
                    }
                }
                else if(ipBarResult == 2){ //client pressed escape
                    SDL_StopTextInput(pGame->pWindow);
                    destroyIpBar(pGame->pIpBar);
                    pGame->pIpBar = NULL;
                    pGame->state = MENU;
                }
                break;

            case LOBBY:
                SDL_StartTextInput(pGame->pWindow);
                lobbyNameResult = lobbyNameInputHandle(pGame->pLobby, &event);
                if(lobbyNameResult == 1){
                    sendName(pGame->pClientNet, returnName(pGame->pLobby));
                    SDL_StopTextInput(pGame->pWindow);
                }
                break;
                
                
                
                
            default: break;
        }  
    }
}

void renderGame(Game *pGame){
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);

    switch(pGame->state){
        case MENU:
            renderMenu(pGame->pMenu);
            break;
        case ENTER_IP:
            renderIpBar(pGame->pIpBar);
            break;
        case LOBBY:
            if(!isDoneTypingName(pGame->pLobby)){
                renderNameInput(pGame->pLobby);
            }
            renderLobby(pGame->pLobby);
            
            

        default: break;
    }
    SDL_RenderPresent(pGame->pRenderer);

    
}

void updateGame(Game *pGame){

    if(pGame->pServerNet){
        acceptClients(pGame->pServerNet);
        messageBuffer(pGame->pServerNet);
    }
    char packet[BUFFERSIZE+1] = {0};
    int result = readFromServer(pGame->pClientNet, packet, BUFFERSIZE);

    if(result< 0){
        pGame->pClientNet = NULL;
        pGame->state = MENU;
        return;
    }
    switch(packet[0]){
        case MSG_NAME:
            lobbyAddPlayer(pGame->pLobby, &packet[1]);
            break;
        
        case MSG_READY:
            //TODO
            break;
        
        default: break;
    }
}

void run(Game *pGame){ 

    while(pGame->isRunning){
        handleInput(pGame);
        updateGame(pGame);
        renderGame(pGame);
      
    }

}

void close(Game *pGame){

    if(pGame->pLobby) destroyLobby(pGame->pLobby);
    if(pGame->pServerNet) destroyServerNetwork(pGame->pServerNet);
    if(pGame->pClientNet) destroyClientNetwork(pGame->pClientNet);
    if(pGame->pIpBar) destroyIpBar(pGame->pIpBar);
    if(pGame->pMenu) destroyMenu(pGame->pMenu);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    TTF_Quit(); 
    NET_Quit();
    SDL_Quit();
}