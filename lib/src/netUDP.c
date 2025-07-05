#include "netUDP.h"
#include <SDL3_net/SDL_net.h>
#include <stdlib.h>
#include <string.h>
#include "stateAndData.h"
#include <SDL3/SDL_stdinc.h>
#include "game.h"

#define PORTUDP 8182

struct serverUDP{
    NET_Address *clientAddresses[MAXCLIENTS];
    NET_DatagramSocket *srv_sock;
    NET_Datagram *datagramFromClient;
    int nrOfClients;
    int UDPhandshakeReceived; // only for tcp->transition
    GameCoreData gData;

};

struct clientUDP{
    NET_Address *pAddress;
    int clientIndex;
    NET_DatagramSocket *cli_sock;
    NET_Datagram *datagramFromServer;
};



ServerUDP *createUDPServer(int nrOfClients){
    ServerUDP *pSrvUDP = malloc(sizeof(struct serverUDP));
    SDL_memset(pSrvUDP->clientAddresses, 0, sizeof(pSrvUDP->clientAddresses));
    pSrvUDP->datagramFromClient = NULL;

    pSrvUDP->srv_sock = NET_CreateDatagramSocket(NULL, PORTUDP);
    if(!pSrvUDP->srv_sock){printf("Error udp sock init %s: \n", SDL_GetError()); return NULL;}
    pSrvUDP->nrOfClients = nrOfClients;
    pSrvUDP->UDPhandshakeReceived = 0;
    SDL_memset(&pSrvUDP->gData, 0, sizeof(GameCoreData));


    
    return pSrvUDP;
}

ClientUDP *createUDPClient(char *ipString, int index){
    ClientUDP *pCliUDP = malloc(sizeof(struct clientUDP));
    pCliUDP->pAddress = NET_ResolveHostname(ipString);
    if(!pCliUDP->pAddress){printf("Error address resolve UDP %s: \n", SDL_GetError()); return NULL;}
    if(NET_WaitUntilResolved(pCliUDP->pAddress, 500) != 1){
        printf("Resolution fail %s: \n", SDL_GetError()); 
        destroyUDPClient(pCliUDP);
        return NULL;
    }
    pCliUDP->cli_sock = NET_CreateDatagramSocket(pCliUDP->pAddress, 0);
    if(!pCliUDP->cli_sock) {printf("error udp cli sock init %s: \n", SDL_GetError()); destroyUDPClient(pCliUDP); return NULL;};
    pCliUDP->clientIndex = index;
 
    return pCliUDP;
}


void sendClientInfoToUDP(ClientUDP *pCliUDP){
    char buf[1+1] = {0};
    buf[0] = MSG_CLIENT_INFO;
    buf[1] = pCliUDP->clientIndex;
    NET_SendDatagram(pCliUDP->cli_sock, pCliUDP->pAddress, PORTUDP, buf, sizeof(buf));
}

GameCoreData *get_gDataUDP(ServerUDP *pSrvUDP){
    return &pSrvUDP->gData;
}

void sendWPMtoUDP(ClientUDP *pCliUDP, char *wpm){
    char buf[1 + 128] = {0};
    buf[0] = MSG_WPM;
    buf[1] = pCliUDP->clientIndex;
    strcpy(&buf[2], wpm);
    printf("Client %d sending WPM: %s\n", pCliUDP->clientIndex+1, wpm);
    NET_SendDatagram(pCliUDP->cli_sock, pCliUDP->pAddress, PORTUDP, buf, sizeof(buf));
}

int readFromClientsUDP(ServerUDP *pSrvUDP){
    if (!pSrvUDP || !pSrvUDP->srv_sock) return 0;
    pSrvUDP->datagramFromClient = NULL;
    while(NET_ReceiveDatagram(pSrvUDP->srv_sock, &pSrvUDP->datagramFromClient) && pSrvUDP->datagramFromClient != NULL){
        int clientIndex; 
        switch(pSrvUDP->datagramFromClient->buf[0]){
            case MSG_CLIENT_INFO:
                clientIndex = pSrvUDP->datagramFromClient->buf[1];
                pSrvUDP->clientAddresses[clientIndex] = NET_RefAddress(pSrvUDP->datagramFromClient->addr);
                pSrvUDP->UDPhandshakeReceived++;
                printf("UDP server got the client's %d address: %s \n", clientIndex+1, NET_GetAddressString(pSrvUDP->datagramFromClient->addr));
                if(pSrvUDP->UDPhandshakeReceived == pSrvUDP->nrOfClients) return 1;
                break;
            case MSG_WPM:
                clientIndex = pSrvUDP->datagramFromClient->buf[1];
                strcpy(pSrvUDP->gData.players[clientIndex].WPM, (const char*)&pSrvUDP->datagramFromClient->buf[2]);
                printf("Server received WPM from client %d: %s\n", clientIndex+1, pSrvUDP->gData.players[clientIndex].WPM);
                writeToUDPClients(pSrvUDP);
                break;
        }
        NET_DestroyDatagram(pSrvUDP->datagramFromClient);
        pSrvUDP->datagramFromClient = NULL;
               
    }             

    return 0;
}

void readFromServerUDP(ClientUDP *pCliUDP, GameCore *pCore){
    pCliUDP->datagramFromServer = NULL;
    while(NET_ReceiveDatagram(pCliUDP->cli_sock, &pCliUDP->datagramFromServer) && pCliUDP->datagramFromServer!=NULL){
        switch(pCliUDP->datagramFromServer->buf[0]){
            case MSG_WPM:
                SDL_memcpy(getGData_local(pCore), &pCliUDP->datagramFromServer->buf[1], sizeof(GameCoreData));
                setGameCoreChanged(pCore, true);
                /* DEBUG: print the WPM values that were just stored */
                GameCoreData *gd = getGData_local(pCore);
                for (int i = 0; i < gd->nrOfPlayers; ++i) {
                    printf("Player %d WPM = %s\n", i + 1, gd->players[i].WPM);
                }
                break;
        }
    }
}

void writeToUDPClients(ServerUDP *pSrvUDP){
    char buf[1+ sizeof(GameCoreData)];
    buf[0] = MSG_WPM;
    SDL_memcpy(&buf[1], &pSrvUDP->gData,sizeof(GameCoreData));
    for(int i = 0; i < pSrvUDP->gData.nrOfPlayers;i++){
        NET_SendDatagram(pSrvUDP->srv_sock, pSrvUDP->clientAddresses[i], 0, buf, sizeof(buf));
        printf("Send MSG_WPM to player %d\n", i);
    }
}


void destroyUDPServer(ServerUDP *pSrvUDP){
    for(int i = 0; i < MAXCLIENTS; i++){
        if (pSrvUDP->clientAddresses[i])
        NET_UnrefAddress(pSrvUDP->clientAddresses[i]);
    }
    if(pSrvUDP->srv_sock) NET_DestroyDatagramSocket(pSrvUDP->srv_sock);
    if(pSrvUDP->datagramFromClient) NET_DestroyDatagram(pSrvUDP->datagramFromClient);
    free(pSrvUDP);
}
void destroyUDPClient(ClientUDP *pCliUDP){
    if(pCliUDP->datagramFromServer) NET_DestroyDatagram(pCliUDP->datagramFromServer);
    if(pCliUDP->cli_sock) NET_DestroyDatagramSocket(pCliUDP->cli_sock);
    if(pCliUDP->pAddress) NET_UnrefAddress(pCliUDP->pAddress);
    free(pCliUDP);

}