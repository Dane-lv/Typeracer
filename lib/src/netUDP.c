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

ClientUDP *createUDPClient(char *ipString){
    ClientUDP *pCliUDP = malloc(sizeof(struct clientUDP));
    pCliUDP->pAddress = NET_ResolveHostname(ipString);
    if(!pCliUDP->pAddress){printf("Error address resolve UDP %s: \n", SDL_GetError()); return NULL;}
    if(NET_WaitUntilResolved(pCliUDP->pAddress, 500) != 1){
        printf("Resolution fail %s: \n", SDL_GetError()); 
        destroyUDPClient(pCliUDP);
        return NULL;
    }
 
    return pCliUDP;
}


void destroyUDPServer(ServerUDP *pSrvUDP){
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