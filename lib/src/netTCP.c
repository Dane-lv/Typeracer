#include "netTCP.h"
#include <SDL3_net/SDL_net.h>
#include <stdlib.h>
#include "lobby.h"
#include "stateAndData.h"
#include <SDL3/SDL_stdinc.h>
#define IP "127.0.0.1" 
#define PORT 8181
#define MAXCLIENTS 4

struct server{
    NET_Server *srv_sock; // just listens and accepts clients
    NET_StreamSocket *cli_sock[MAXCLIENTS]; // srv -> cli
    int nrOfClients;
    LobbyData lobbyData;
    GameCoreData gData;
    bool playersReady;
};

struct client{
    NET_StreamSocket *cli; // cli -> srv
    char ipString[15];
    NET_Address *pAddress;
    int clientIndex;
};

Server *createServer(){
    Server *pSrv = malloc(sizeof(struct server));
    pSrv->srv_sock = NET_CreateServer(NULL, PORT);
    if(!pSrv->srv_sock){printf("Error srv sock init %s: \n", SDL_GetError()); return NULL;}
    memset(pSrv->cli_sock, 0, sizeof(pSrv->cli_sock));
    pSrv->nrOfClients = 0;
    printf("Server started on port %d\n", PORT);
    memset(&pSrv->lobbyData, 0, sizeof(LobbyData));
    memset(&pSrv->gData, 0, sizeof(GameCoreData));
    pSrv->playersReady = false;
    return pSrv;
}

Client *createClient(char *ipString, int port){
    
    Client *pCli = malloc(sizeof(struct client));
    SDL_strlcpy(pCli->ipString, ipString, 15); // save ip address for future?
    pCli->pAddress = NET_ResolveHostname(ipString);
    if(!pCli->pAddress){printf("Error address resolve %s: \n", SDL_GetError()); return NULL;}
    if(NET_WaitUntilResolved(pCli->pAddress, 500) != 1){
        printf("Resolution fail %s: \n", SDL_GetError()); 
        destroyClient(pCli);
        return NULL;
    }
    pCli->cli = NET_CreateClient(pCli->pAddress, port);
    if(!pCli->cli){printf("Error cli sock init %s: \n", SDL_GetError()); return NULL;}
    if(!NET_WaitUntilConnected(pCli->cli, 1000)){ printf("Error: Host client failed to connect to server\n");destroyClient(pCli);return NULL;}
  
    return pCli;
}

char *getIpString(Client *pCli){
    return pCli->ipString;
}

void acceptClients(Server *pSrv){
    if(pSrv == NULL) return;
    while(pSrv->nrOfClients <= MAXCLIENTS){
        NET_StreamSocket *pending_sock = NULL;
        if(!NET_AcceptClient(pSrv->srv_sock, &pending_sock) || pending_sock == NULL){
            break;
        }
        pSrv->cli_sock[pSrv->nrOfClients] = pending_sock;
        // SEND PLAYER ITS INDEX
        char buf[2];
        buf[0] = MSG_PLAYER_INDEX;
        buf[1] = pSrv->nrOfClients;
        NET_WriteToStreamSocket(pSrv->cli_sock[pSrv->nrOfClients], buf, sizeof(buf));
        // 
        pSrv->nrOfClients++;
        printf("Client %d connected!\n", pSrv->nrOfClients);
        if(pSrv->nrOfClients == 1){
            pSrv->lobbyData.players[0].isHost = true;
        }
        else{
            pSrv->lobbyData.players[pSrv->nrOfClients-1].isHost = false;
        }
       
    }
}

void sendGameStart(Client *pCli){
    char buf[1] = {0};
    buf[0] = MSG_START_GAME;
    NET_WriteToStreamSocket(pCli->cli, buf, sizeof(buf));
}

void sendPlayerName(Client *pCli, char *playerName){
    char buf[1 + MAXNAME] = {0}; // +1 for the message type
    buf[0] = MSG_NAME; 
    SDL_strlcpy(&buf[1], playerName, MAXNAME);
    NET_WriteToStreamSocket(pCli->cli, buf, sizeof(buf));
}

void sendPlayerStatus(Client *pCli, bool status){
    char buf[1 + 1] = {0}; 
    buf[0] = MSG_READY;
    if(status == true){
        buf[1] = 1;
    }
    else buf[1] = 0;
    NET_WriteToStreamSocket(pCli->cli, buf, sizeof(buf));
}

void readFromClients(Server *pSrv){
    if(pSrv == NULL) return;

    for(int i = 0; i < pSrv->nrOfClients; i++)
    {
        if(pSrv->cli_sock[i] != NULL){
            char buf[1 + MAXNAME]; // [MSG_NAME][NAME];
            int bytesRead = NET_ReadFromStreamSocket(pSrv->cli_sock[i], buf, sizeof(buf));
            if(bytesRead < 0){
                printf("Client %d connection failed: %s\n", i+1, SDL_GetError());
                disconnectPlayer(pSrv, i);
                i--; // recheck the shifted client at index i.
                writeToClients(pSrv);
                continue;
            }
            else if(bytesRead == 0){ // NO DATA, NEXT CLIENT
                continue;
            }
            else{
                switch(buf[0]){
                    case MSG_NAME:
                        SDL_strlcpy(pSrv->lobbyData.players[i].playerName, &buf[1], MAXNAME); 
                        pSrv->lobbyData.players[i].isReady = false; // All players are not ready when joining
                        pSrv->lobbyData.nrOfPlayers++;
                        printf("Player %d joined: %s (ready: no)\n", i+1, pSrv->lobbyData.players[i].playerName);
                        break;
                    case MSG_READY:
                        if( pSrv->lobbyData.players[i].isReady == true){
                            pSrv->lobbyData.players[i].isReady = false;
                            printf("Player %d %s is not ready\n", i+1, pSrv->lobbyData.players[i].playerName);
                        }
                        else{pSrv->lobbyData.players[i].isReady = true;  printf("Player %d %s is ready\n", i+1, pSrv->lobbyData.players[i].playerName);}
                        break;
                    case MSG_START_GAME:
                        copyNamesToGameCore(pSrv); // COPY NAMES AND NROFCLIENTS, SEND TO CLIENTS AFTER
                        pSrv->playersReady = true; // send it to all players
                        break;
                        
                    default: break;
                }
                writeToClients(pSrv);
            }
        }
    }
}

void copyNamesToGameCore(Server *pSrv){
    for(int i = 0; i < pSrv->nrOfClients; i++){
        SDL_strlcpy(pSrv->gData.players[i].playerName, pSrv->lobbyData.players[i].playerName, sizeof(pSrv->gData.players[i].playerName));
    }
    pSrv->gData.nrOfPlayers = pSrv->nrOfClients;
}

void disconnectPlayer(Server *pSrv, int playerIndex){
    NET_DestroyStreamSocket(pSrv->cli_sock[playerIndex]);     

    for(int i = playerIndex; i < pSrv->nrOfClients-1; i++){
        pSrv->cli_sock[i] = pSrv->cli_sock[i+1];
        pSrv->lobbyData.players[i] = pSrv->lobbyData.players[i+1];
    }
    pSrv->cli_sock[pSrv->nrOfClients - 1] = NULL;
    memset(&pSrv->lobbyData.players[pSrv->nrOfClients - 1], 0, sizeof(ClientData));

    pSrv->nrOfClients--;
    pSrv->lobbyData.nrOfPlayers--;
}

void writeToClients(Server *pSrv){

    if(!pSrv->playersReady){
        char buf[1 + sizeof(LobbyData)];
        buf[0] = MSG_LOBBY;
        SDL_memcpy(&buf[1], &pSrv->lobbyData, sizeof(LobbyData));
        for(int i = 0; i < pSrv->nrOfClients; i++){
            if(pSrv->cli_sock[i]){
                NET_WriteToStreamSocket(pSrv->cli_sock[i], buf, sizeof(buf));
            }
        }
    }
    else if(pSrv->playersReady == true){ // Send it to all players when host has started the game
        char buf[1] = {0};
        buf[0] = MSG_START_GAME;
        for(int i = 0; i < pSrv->nrOfClients; i++){
            if(pSrv->cli_sock[i]){
                NET_WriteToStreamSocket(pSrv->cli_sock[i], buf, sizeof(buf));
            }
        }
    }
}

void readFromServer(Client *pCli, Lobby *pLobby){
    if(pCli == NULL || pLobby == NULL) return;
    char buf[1 + sizeof(LobbyData)];
    int bytesRead = NET_ReadFromStreamSocket(pCli->cli, buf, sizeof(buf));
    if(bytesRead == -1){
        printf("Server crashed %s: \n", SDL_GetError());
        NET_DestroyStreamSocket(pCli->cli);
        pCli->cli = NULL;
        return;
    }
    else if(bytesRead == 0){return;}    // NO DATA, RETURN;
    else{

        switch (buf[0]){
            case MSG_LOBBY:
                SDL_memcpy(getLobbyLocal(pLobby), &buf[1], sizeof(LobbyData)); // same as memcpy
                setLobbyChanged(pLobby,true); 
                break;
            case MSG_PLAYER_INDEX: // GET INDEX FROM SERVER UPON JOINING
                pCli->clientIndex = buf[1];
                break;
            case MSG_START_GAME: // make tcp send info about clients to udp
                break;

            default: break;
        }
    }
}



void destroyClient(Client *pCli){
    if(pCli->cli) NET_DestroyStreamSocket(pCli->cli);
    if(pCli->pAddress) NET_UnrefAddress(pCli->pAddress);
    free(pCli);
}



void destroyServer(Server *pSrv){
    for(int i = 0; i < pSrv->nrOfClients; i++){
        NET_DestroyStreamSocket(pSrv->cli_sock[i]);
    }
    if(pSrv->srv_sock) NET_DestroyServer(pSrv->srv_sock);
    free(pSrv);
}

