#include "netUDP.h"
#include <SDL3_net/SDL_net.h>
#include <stdlib.h>
#include "stateAndData.h"
#include <SDL3/SDL_stdinc.h>

#define PORTUDP 8182

struct serverUDP{
    NET_Address *clientAddresses[MAXCLIENTS];
    NET_DatagramSocket *srv_sock;
    NET_Datagram *datagramFromClient;

};

struct clientUDP{
    NET_Address *pAddress;
    int clientIndex;
    NET_DatagramSocket *cli_sock;
    NET_Datagram *datagramFromServer;
};



ServerUDP *createUDPServer(){
    ServerUDP *pSrvUDP = malloc(sizeof(struct serverUDP));
    SDL_memset(pSrvUDP->clientAddresses, 0, sizeof(pSrvUDP->clientAddresses));
    pSrvUDP->datagramFromClient = NULL;

    pSrvUDP->srv_sock = NET_CreateDatagramSocket(NULL, PORTUDP);
    if(!pSrvUDP->srv_sock){printf("Error udp sock init %s: \n", SDL_GetError()); return NULL;}


    
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
    SDL_Delay(444);
}

void readFromClientsUDP(ServerUDP *pSrvUDP){
    if (!pSrvUDP || !pSrvUDP->srv_sock) return;
    pSrvUDP->datagramFromClient = NULL;
    while(NET_ReceiveDatagram(pSrvUDP->srv_sock, &pSrvUDP->datagramFromClient) && pSrvUDP->datagramFromClient != NULL){
        int clientIndex;
        switch(pSrvUDP->datagramFromClient->buf[0]){
            case MSG_CLIENT_INFO:
                clientIndex = pSrvUDP->datagramFromClient->buf[1];
                pSrvUDP->clientAddresses[clientIndex] = NET_RefAddress(pSrvUDP->datagramFromClient->addr);
                printf("UDP server got the client's %d address: %s \n", clientIndex+1, NET_GetAddressString(pSrvUDP->datagramFromClient->addr));
                break;
        }
        NET_DestroyDatagram(pSrvUDP->datagramFromClient);
        pSrvUDP->datagramFromClient = NULL;
               
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