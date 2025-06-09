#include <SDL3_net/SDL_net.h>
#include <stdlib.h>
#include "network.h"
#include <string.h>
#include "stateAndData.h"



struct clientNetwork{
    NET_StreamSocket *pSocket; // connection to the server
    NET_Address *pAdress; // passed by ipString from a client
    char ipString[64];
    int port; // port is predefined in stateanddata.h
};

struct serverNetwork{
    NET_Server *pServer; // listening for incoming connection on a predefined port
    NET_StreamSocket *pClients[MAXPLAYERS]; // array of active server connection
    int nrOfClients;
    int port; // port is predefined in stateanddata.h
};

ClientNetwork *createClientNetwork(char *ipString, int port){
    ClientNetwork *pClientNet = malloc(sizeof(struct clientNetwork));
    if(!pClientNet){
        printf("Error client netwrok malloc");
        return NULL;
    }
    pClientNet->pSocket = NULL;
    pClientNet->pAdress = NULL;
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
    for(int i = 0;i<MAXPLAYERS;i++){
        if(pServerNet->pClients[i]){
            NET_DestroyStreamSocket(pServerNet->pClients[i]);
        }
    }
    if(pServerNet->pServer) NET_DestroyServer(pServerNet->pServer);
    free(pServerNet);
}

void acceptClients(ServerNetwork *pServerNet){
    if (pServerNet->nrOfClients >= MAXPLAYERS) return;
    NET_StreamSocket *pClient = NULL; // Gets an address of a socket that gets created when a client connects
    while(NET_AcceptClient(pServerNet->pServer, &pClient)){
        pServerNet->pClients[pServerNet->nrOfClients++] = pClient;
        printf("Client connected\n");
        if(pServerNet->nrOfClients == MAXPLAYERS) break;
    }
}

int readFromServer(ClientNetwork *pClient, char *packet, int bufferSize){ // Client reads info from the server
    char packetReceive[BUFFERSIZE+1] = {0};
    int bytesRead = NET_ReadFromStreamSocket(pClient->pSocket,packetReceive,bufferSize);
    if(bytesRead < 0){
        printf("Server crashed\n");
        destroyClientNetwork(pClient);
        return -1;
    }
    if(bytesRead > bufferSize) bytesRead = bufferSize;
    memcpy(packet,packetReceive, bytesRead);
    packet[bytesRead] = '\0'; 

    return bytesRead;

}

void sendName(ClientNetwork *pClientNet,  char *name) // Sends name to the server.
{
    if(!pClientNet || !pClientNet->pSocket) {
        printf("Error: Invalid client socket\n");
        return;
    }
    
    char packet[MAXNAME] = {0};              
    packet[0] = MSG_NAME;
    strncpy(&packet[1], name, MAXNAME - 2);      
    NET_WriteToStreamSocket(pClientNet->pSocket, packet, MAXNAME);
} 

void messageBuffer(ServerNetwork *pServerNet){ // Reads incoming packets from each client and broadcasts to all clients.
    for(int i = 0; i < pServerNet->nrOfClients; i++){
        NET_StreamSocket *pSocket = pServerNet->pClients[i];

        if(pSocket){
            char packetReceive[BUFFERSIZE+1] = {0};
            int bytesRead = NET_ReadFromStreamSocket(pSocket,packetReceive,BUFFERSIZE);
            if(bytesRead < 0){
                printf("Client %d disconnected\n", i);
                NET_DestroyStreamSocket(pSocket);
                pServerNet->pClients[i] = NULL;
                pServerNet->nrOfClients--;
            } else{
                if(packetReceive[0] == MSG_NAME){
                    printf("Server got name: %s\n", &packetReceive[1]);
                    for(int j = 0; j < MAXPLAYERS; j++){
                        if(pServerNet->pClients[j]){ // Array of connected client sockets
                            NET_WriteToStreamSocket(pServerNet->pClients[j], packetReceive, MAXNAME);
                        }
                        


                    }
                }
            }
            
        }
    }
}


void destroyClientNetwork(ClientNetwork *pClientNet){
    if(pClientNet->pSocket) NET_DestroyStreamSocket(pClientNet->pSocket);
    if(pClientNet->pAdress) NET_UnrefAddress(pClientNet->pAdress);
    free(pClientNet);
}