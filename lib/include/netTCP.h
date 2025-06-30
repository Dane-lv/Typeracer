#ifndef netTCP_h
#define netTCP_h
#include <stdbool.h>

typedef struct server Server;
typedef struct client Client;
typedef struct lobby Lobby;
typedef struct gameCore GameCore;


int getIndex(Client *pCli);


Server *createServer();
void destroyServer(Server *pSrv);
void acceptClients(Server *pSrv);
int getNrOfClients(Server *pSrv);
Client *createClient(char *ipString, int port);
void destroyClient(Client *pCli);
void sendPlayerName(Client *pCli, char *playerName);
void readFromClients(Server *pSrv);
void writeToClients(Server *pSrv);
void readFromServer(Client *pCli, Lobby *pLobby);
void disconnectPlayer(Server *pSrv, int playerIndex);
void sendPlayerStatus(Client *pCli, bool status);
void sendGameStart(Client *pCli);
void copyDataToGameCore(Server *pSrv);
char *getIpString(Client *pCli);
bool isGameStarted(Client *pCli);
bool playersAreReady(Server *pSrv);
void copyDataToGameCoreClient(Client *pCli, GameCore *pCore);



#endif