#ifndef netudp_h
#define netudp_h
#include "game.h"

typedef struct serverUDP ServerUDP;
typedef struct clientUDP ClientUDP;
typedef struct gameCore GameCore;


ServerUDP *createUDPServer(int nrOfClients);
ClientUDP *createUDPClient(char *ipString, int index);
void destroyUDPServer(ServerUDP *pSrvUDP);
void destroyUDPClient(ClientUDP *pCliUDP);

void sendClientInfoToUDP(ClientUDP *pCliUDP);
int readFromClientsUDP(ServerUDP *pSrvUDP);
void sendWPMtoUDP(ClientUDP *pCliUDP, char *wpm, int currentWordIndex);
void readFromServerUDP(ClientUDP *pCliUDP, GameCore *pCore);
void writeToUDPClients(ServerUDP *pSrvUDP);
GameCoreData *get_gDataUDP(ServerUDP *pSrvUDP);
char *checkPlacements(ServerUDP *pSrvUDP, int clientIndex);
void sendTextFinished(ClientUDP *pCliUDP);


#endif