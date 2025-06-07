#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include <stdlib.h>
#include "network.h"
#include <string.h>
#include "stateAndData.h"

#define BUFFERSIZE 512
#define MAX_CLIENTS 4

struct clientNetwork{
    NET_StreamSocket *pSocket; //client socket
    NET_Address *pAdress; 
    char ipString[64]; 
    int port;
};

struct serverNetwork{
    NET_Server *pServer;
    NET_StreamSocket *pClients[MAX_CLIENTS];
    int nrOfClients;
    int port;
};

ClientNetwork *createClientNetwork(char *ipString, int port){
    ClientNetwork *pClientNet = malloc(sizeof(struct clientNetwork));
    if(!pClientNet){
        return NULL;
    }
    pClientNet->port = port;
    strcpy(pClientNet->ipString, ipString);
    pClientNet->pAdress = NET_ResolveHostname(ipString);
    if(!pClientNet->pAdress){
        printf("Resolve hostname fail: %s\n", SDL_GetError());
        destroyClientNetwork(pClientNet);
        return NULL;    
    }
    if (NET_WaitUntilResolved(pClientNet->pAdress, 500) != 1) {
        printf("Host resolution timeout/failure: %s\n", SDL_GetError());
        destroyClientNetwork(pClientNet);
        return NULL;
    }
    return pClientNet;

}

ServerNetwork *createServerNetwork(int port){
    ServerNetwork *pServerNet = malloc(sizeof(struct serverNetwork));
    if (!pServerNet) return NULL;
    pServerNet->pServer = NET_CreateServer(NULL, port);
    if(!pServerNet->pServer){
        printf("Error creating server: %s\n", SDL_GetError());
        destroyServerNetwork(pServerNet);
        return NULL;
    }
    pServerNet->nrOfClients = 0;
    pServerNet->port = port;
    memset(pServerNet->pClients, 0, sizeof(pServerNet->pClients));
    return pServerNet;
}


int connectToServer(ClientNetwork *pClientNet){
    pClientNet->pSocket = NET_CreateClient(pClientNet->pAdress, pClientNet->port);
    if(!pClientNet->pSocket) return 0;
    return 1;
}

int holdUntilConnected(ClientNetwork *pClientNet, int timeout){
    if (!pClientNet || !pClientNet->pSocket) return -1;
    return NET_WaitUntilConnected(pClientNet->pSocket, timeout);
}

void destroyServerNetwork(ServerNetwork *pServerNet){
    for(int i = 0;i<MAX_CLIENTS;i++){
        if(pServerNet->pClients[i]){
            NET_DestroyStreamSocket(pServerNet->pClients[i]);
        }
    }
    if(pServerNet->pServer) NET_DestroyServer(pServerNet->pServer);
    free(pServerNet);
}

void acceptClients(ServerNetwork *pServerNet){
    if (pServerNet->nrOfClients >= MAX_CLIENTS) return;
    NET_StreamSocket *pClient = NULL;
    while(NET_AcceptClient(pServerNet->pServer, &pClient)){
        if(!pClient) break;
        pServerNet->pClients[pServerNet->nrOfClients++] = pClient;
        printf("Client connected\n");
        if(pServerNet->nrOfClients == MAX_CLIENTS) break;
    }
}

void sendName(ClientNetwork *pClientNet,  char *name)
{
    if(!pClientNet || !pClientNet->pSocket) {
        printf("Error: Invalid client socket\n");
        return;
    }
    
    char packet[MAXNAME] = {0};              
    packet[0] = MSG_NAME;
    strcpy(&packet[1], name);      
    NET_WriteToStreamSocket(pClientNet->pSocket, packet, MAXNAME);
}

void messageBuffer(ServerNetwork *pServerNet){
    for(int i = 0; i < pServerNet->nrOfClients; i++){
        NET_StreamSocket *pSocket = pServerNet->pClients[i];

        if(pSocket){
            char packetReceive[BUFFERSIZE+1] = {0};
            int bytesRead = NET_ReadFromStreamSocket(pSocket,packetReceive,BUFFERSIZE);
            
            if(packetReceive[0] == MSG_NAME){
                printf("Server got name %s: \n", &packetReceive[1]);
            }
            else if(bytesRead == -1){
                printf("Client %d disconnected\n", i);
                NET_DestroyStreamSocket(pSocket);
                pServerNet->pClients[i] = NULL;
                pServerNet->nrOfClients--;
            }
        }
    }
}


void destroyClientNetwork(ClientNetwork *pClientNet){
    if(pClientNet->pSocket) NET_DestroyStreamSocket(pClientNet->pSocket);
    if(pClientNet->pAdress) NET_UnrefAddress(pClientNet->pAdress);
    free(pClientNet);
}