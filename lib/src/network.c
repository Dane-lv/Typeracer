#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include <stdlib.h>
#include "network.h"
#include <string.h>

struct network{
    NET_StreamSocket *pSocket;
    NET_Address *pAdress;
    char ipString[64];
    int port;

};

Network *createNetwork(char *ipString, int port){
    Network *pNetwork = malloc(sizeof(struct network));
    if(!pNetwork){
        return NULL;
    }
    pNetwork->port = port;
    strcpy(pNetwork->ipString, ipString);
    pNetwork->pAdress = NET_ResolveHostname(ipString);
    if(!pNetwork->pAdress){
        printf("Resolve hostname fail: %s\n", SDL_GetError());
        destroyNetwork(pNetwork);
        return NULL;    
    }
    if(!NET_WaitUntilResolved(pNetwork->pAdress, -1)){
        printf("Resolve failed: %s\n", SDL_GetError());
        destroyNetwork(pNetwork);
        return NULL;
    }
    return pNetwork;

}

int connectToServer(Network *pNetwork){
    pNetwork->pSocket = NET_CreateClient(pNetwork->pAdress, pNetwork->port);
    if(!pNetwork->pSocket) return 0;
    return 1;
}

void destroyNetwork(Network *pNetwork){
    
    if(pNetwork->pSocket) NET_DestroyStreamSocket(pNetwork->pSocket);
    if(pNetwork->pAdress) NET_UnrefAddress(pNetwork->pAdress);
    free(pNetwork);
}