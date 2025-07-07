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
#include "main.h"
#include "stateAndData.h"
#include "netTCP.h"
#include "menu.h"
#include "lobby.h"
#include "netUDP.h"
#include "game.h"

struct game {
    SDL_Renderer *pRenderer;
    SDL_Window *pWindow;
    bool isRunning;
    GameState state;
    Menu *pMenu;
    IpBar *pIpBar;
    Server *pSrv; //TCP
    Client *pCli;   //TCP
    ServerUDP *pSrvUDP;
    ClientUDP *pCliUDP;
    GameCore *pCore;
    
    Lobby *pLobby;

};
typedef struct game Game;

bool init(Game *pGame);
void close(Game *pGame);
void handleInput(Game *pGame);
void updateGame(Game *pGame);
void renderGame(Game *pGame);
void run(Game *pGame);




int main(){
    srand(time(NULL));
    Game g = {0};
    if(!init(&g)) {return 1;}
    run(&g);
    close(&g);
    return 0;
}

void run(Game *pGame){

    while(pGame->isRunning){
        handleInput(pGame);
        updateGame(pGame);
        renderGame(pGame);
    }
    return;
}

bool init(Game *pGame){
    if (!SDL_Init(SDL_INIT_VIDEO)){ printf("Error SDL Init: %s\n", SDL_GetError()); return false;}
    if (!TTF_Init()){ printf("Error: %s\n",SDL_GetError()); SDL_Quit(); return false;}
    if (!NET_Init()){ printf("Error Net init: %s\n", SDL_GetError()); TTF_Quit(); SDL_Quit(); return false;}

    pGame->pWindow = SDL_CreateWindow("Typeracer", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
    if(!pGame->pWindow){ printf("Error intializing window: %s\n", SDL_GetError()); close(pGame); return false;}   
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, NULL);
    if(!pGame->pRenderer){ printf("Error initializing renderer: %s\n", SDL_GetError()); close(pGame); return false;}

    pGame->isRunning = true;
    pGame->pIpBar = NULL;
    pGame->pSrv = NULL;
    pGame->pCli = NULL;
    pGame->pLobby = NULL;
    pGame->pSrvUDP = NULL;
    pGame->pCliUDP = NULL;
    pGame->pCore = NULL;
    pGame->pMenu = createMenu(pGame->pWindow, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!pGame->pMenu){printf("%s\n", SDL_GetError()); close(pGame); return false;}
    pGame->state = MENU;



    return true;
}

void handleInput(Game *pGame){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_EVENT_QUIT){
            pGame->isRunning = false;
            return;
        }
        int menuOptionsResult, ipInputResult, nameInputResult, playerIsReady, gameCoreInput;
        switch(pGame->state){
            case MENU:
                menuOptionsResult = menuOptionsEvent(pGame->pMenu, &event);
                if(menuOptionsResult == 1){ // clicked connect
                    pGame->pIpBar = createIpBar(pGame->pWindow, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
                    SDL_StartTextInput(pGame->pWindow);
                    pGame->state = IP_INPUT;
                    break;
                }
                else if(menuOptionsResult == 2){  // clicked host game
                    pGame->pSrv = createServer(); /* CREATING NETWORK */
                    if(!pGame->pSrv){printf("Error server create %s:\n", SDL_GetError()); return;}  
                    pGame->pCli = createClient("127.0.0.1", PORT);
                    if(!pGame->pCli) {printf("Error client create (host) %s:\n", SDL_GetError()); return;}
                    bool isHost = true;
                    pGame->pLobby = createLobby(pGame->pRenderer, pGame->pWindow, WINDOW_WIDTH, WINDOW_HEIGHT,isHost);
                    if(!pGame->pLobby) {printf("Error lobby create (host) %s:\n", SDL_GetError()); return;}
                    pGame->state = LOBBY;
                    SDL_StartTextInput(pGame->pWindow);
                    destroyMenu(pGame->pMenu);
                    pGame->pMenu = NULL;
                }
                
                break;
            case IP_INPUT:
                ipInputResult = ipAddressInputHandle(pGame->pIpBar, &event);    
                if(ipInputResult == 1){ // entered a nonempty ip buffer.
                    char *ipString = getIp(pGame->pIpBar);
                    pGame->pCli = createClient(ipString, PORT);   /* CREATING CLIENT*/
                    if(!pGame->pCli){printf("Error client create %s:\n", SDL_GetError()); return;}
                    bool isHost;
                    isHost = false;
                    pGame->pLobby = createLobby(pGame->pRenderer, pGame->pWindow, WINDOW_WIDTH, WINDOW_HEIGHT, isHost);
                    if(!pGame->pLobby){printf("Error lobby create %s:\n", SDL_GetError()); return;}
                    pGame->state = LOBBY;
                    
                }
                else if(ipInputResult == 3){ // pressed escape
                    pGame->state = MENU;
                    destroyIpBar(pGame->pIpBar);
                    pGame->pIpBar = NULL;
                    SDL_StopTextInput(pGame->pWindow);
    
                }
                break;
            case LOBBY:
                if(isStillTyping(pGame->pLobby)){
                    nameInputResult = nameInputHandle(pGame->pLobby, &event);
                    if  (nameInputResult == 1){
                        sendPlayerName(pGame->pCli, getName(pGame->pLobby));
                        SDL_StopTextInput(pGame->pWindow);
                    }
                }
                else{
                    playerIsReady = lobbyEventHandle(pGame->pLobby, &event);
                    if(playerIsReady == 1){ // PLAYER PRESSED SPACE
                        sendPlayerStatus(pGame->pCli, getReadyStatus(pGame->pLobby));
                    }
                    if(playerIsReady == 2){ // HOST PRESSED SPACE

                        if(pGame->pSrvUDP == NULL){
                            if(hostCheck(pGame->pLobby)) {
                                pGame->pSrvUDP = createUDPServer(getNrOfClients(pGame->pSrv));
                                
                            }
                            if(!pGame->pSrvUDP){printf("Erorr udp server create %s: \n", SDL_GetError()); return;}
                        }
                        printf("Host created UDP server\n");

                        sendGameStart(pGame->pCli);

                    }
                }
                break;
            case ONGOING:
                SDL_StartTextInput(pGame->pWindow);
                gameCoreInput = gameCoreInputHandle(pGame->pCore, &event);
                if(gameCoreInput == 1){
                    sendWPMtoUDP(pGame->pCliUDP,getWPM(pGame->pCore), getCurrentWordIndex(pGame->pCore));
                }
                break;
            case ROUND_OVER:
                break;
            default: break;
        }
          
    }
}

void renderGame(Game *pGame){
   SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
   SDL_RenderClear(pGame->pRenderer);

   switch(pGame->state){
        case MENU:
            renderMenu(pGame->pMenu);
            break;
        case IP_INPUT:
            renderIpBar(pGame->pIpBar);
            break;
        case LOBBY:
            renderLobby(pGame->pLobby);
            break;
        case ONGOING:
            renderCore(pGame->pCore);
            break;
        case ROUND_OVER:
            break;
        default:  break;
   }

   SDL_RenderPresent(pGame->pRenderer);

}

void updateGame(Game *pGame){

    switch(pGame->state) {
        case MENU: break;
        case IP_INPUT: break;
        case LOBBY:
            if(pGame->pSrv) 
            {
                acceptClients(pGame->pSrv);
                readFromClients(pGame->pSrv);
                if(playersAreReady(pGame->pSrv) && pGame->pSrvUDP){
                    if(readFromClientsUDP(pGame->pSrvUDP)){
                        printf("UDP handshake lobby SUCCESS\n");               
                    }
                }
            }
            if(pGame->pCli) 
            {
                readFromServer(pGame->pCli, pGame->pLobby);
                if(isGameStarted(pGame->pCli)){
                    if(pGame->pCore == NULL){
                        pGame->pCore = createGameCore(pGame->pWindow, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
                        copyDataToGameCoreClient(pGame->pCli, pGame->pCore);
                        createNamesAndWPM(pGame->pCore);
                    }
                    if(pGame->pCliUDP == NULL){
                        pGame->pCliUDP = createUDPClient(getIpString(pGame->pCli), getIndex(pGame->pCli));
                        if(pGame->pCliUDP){
                            sendClientInfoToUDP(pGame->pCliUDP);
                            printf("Client sent UDP handshake to server\n");
                        }
                    }

                    pGame->state = ONGOING;

                   
                }
            }
            if(pGame->pLobby) 
            {
                updateLobby(pGame->pLobby);
            }
            break;
        case ONGOING: 
            // Continue checking for UDP handshake until all clients connect
            if(pGame->pSrv && pGame->pSrvUDP){
                if(readFromClientsUDP(pGame->pSrvUDP)){
                    printf("UDP handshake ongoing SUCCESS\n");     
                    send_gDataToUDP(pGame->pSrv, pGame->pSrvUDP);          
                }
            }
            if(pGame->pCliUDP){
                readFromServerUDP(pGame->pCliUDP, pGame->pCore);
            }

            updateGameCore(pGame->pCore);
            break;
        case ROUND_OVER: break;
    }
}
    


void close(Game *pGame){
    if(pGame->pCore) destroyGameCore(pGame->pCore);
    if(pGame->pCliUDP) destroyUDPClient(pGame->pCliUDP);
    if(pGame->pSrvUDP) destroyUDPServer(pGame->pSrvUDP);
    if(pGame->pCli) destroyClient(pGame->pCli);
    if(pGame->pLobby) destroyLobby(pGame->pLobby);
    if(pGame->pSrv) destroyServer(pGame->pSrv);
    if(pGame->pIpBar) destroyIpBar(pGame->pIpBar);
    if(pGame->pMenu) destroyMenu(pGame->pMenu);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    TTF_Quit(); 
    NET_Quit();
    SDL_Quit();
}
