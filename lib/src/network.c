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
    int port; // port is predefined in stateanddata.h

    ClientData players[MAXPLAYERS]; // server stores it
    int nrOfPlayers; // live sockets
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
    pServerNet->nrOfPlayers = 0;
    pServerNet->port = port;
    memset(pServerNet->pClients, 0, sizeof(pServerNet->pClients));
    return pServerNet;
}

void broadcastPacket(ServerNetwork *pServerNet, void *buffer, int len, NET_StreamSocket *exclude){
    for(int i = 0; i < pServerNet->nrOfPlayers; i++){
        NET_StreamSocket *pClient = pServerNet->pClients[i];
        if(pClient && pClient != exclude)
            NET_WriteToStreamSocket(pClient, buffer, len);
    }
}

bool isHost(ServerNetwork *pServerNet) {
    return pGame->pServerNet != NULL;
}



void sendLobbyLog(ServerNetwork *pServerNet, NET_StreamSocket *pClient){ // Send server info for every connecting player
    char packet[MAXNAME];

    packet[0] = MSG_NAME;
    for(int i = 0;i < pServerNet -> nrOfPlayers; i++){
        if(pServerNet->players[i].playerName[0] == '\0')
            continue; // Skip blanks

        memcpy(&packet[1], pServerNet->players[i].playerName, MAXNAME-1);
        NET_WriteToStreamSocket(pClient, packet, MAXNAME);
        
    }
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

void acceptClients(ServerNetwork *pServerNet)
{
    NET_StreamSocket *pClient = NULL;

    if(!NET_AcceptClient(pServerNet->pServer, &pClient) || pClient == NULL)
        return;

    if(pServerNet->nrOfPlayers >= MAXPLAYERS){
        NET_DestroyStreamSocket(pClient);
        return;
    }
    int idx = pServerNet->nrOfPlayers;
    pServerNet->pClients[idx] = pClient;
    pServerNet->players[idx].playerName[0] = '\0';
    pServerNet->players[idx].isReady = false;
    pServerNet->nrOfPlayers++;
    sendLobbyLog(pServerNet,pClient);
    printf("Client connected  %d: \n", idx+1); // +1 to include the host.
}   



int readFromServer(ClientNetwork *pClient, char *packet, int bufferSize){ // Client reads info from the server
   
    int bytesRead = NET_ReadFromStreamSocket(pClient->pSocket,packet,bufferSize);
    if(bytesRead < 0){
        printf("Server crashed\n");
        destroyClientNetwork(pClient);
        return -1;
    }

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
    strncpy(&packet[1], name, MAXNAME-1);      
    NET_WriteToStreamSocket(pClientNet->pSocket, packet, MAXNAME);
} 

void messageBuffer(ServerNetwork *pServerNet)   // Send packet to all clients
{
    char packet[MAXNAME];
    for (int i = 0; i < pServerNet->nrOfPlayers; ++i)
    {
        NET_StreamSocket *pClient = pServerNet->pClients[i];
        if (pClient != NULL)
        {
            int bytesRead = NET_ReadFromStreamSocket(pClient, packet, MAXNAME);
            if (bytesRead < 0)
            {
                printf("Client %d disconnected\n", i);
                NET_DestroyStreamSocket(pClient);
                pServerNet->pClients[i] = NULL;
            }
            else
            {
                if (bytesRead >= MAXNAME)
                {
                    switch (packet[0])
                    {
                        case MSG_NAME:
                            strncpy(pServerNet->players[i].playerName, &packet[1], MAXNAME - 1);
                            pServerNet->players[i].playerName[MAXNAME - 1] = '\0';
                            break;

                        case MSG_READY:
                            pServerNet->players[i].isReady = true;    
                            broadcastPacket(pServerNet, packet, sizeof(ReadyPacket), NULL);
                            break;

                        case MSG_START_GAME:
                            if (i == 0 && allPlayersReady(pServerNet)) { // Host is always at index 0
                            broadcastPacket(pServerNet, packet, sizeof(StartGamePacket), NULL);
                            }
                            break;


                        default:
                            break;
                    }

                    broadcastPacket(pServerNet, packet, MAXNAME, pClient);
                }
               
            }
        }
      
    }
}

bool allPlayersReady(ServerNetwork *pServerNet) {
    for (int i = 0; i < pServerNet->nrOfPlayers; i++) {
        if (!pServerNet->players[i].isReady) return false;
    }
    return true;
}



void destroyClientNetwork(ClientNetwork *pClientNet){
    if(pClientNet->pSocket) NET_DestroyStreamSocket(pClientNet->pSocket);
    if(pClientNet->pAdress) NET_UnrefAddress(pClientNet->pAdress);
    free(pClientNet);
}
