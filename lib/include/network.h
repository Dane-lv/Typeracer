#ifndef network_h
#define network_h
#include <stdbool.h>

typedef struct clientNetwork ClientNetwork;
typedef struct serverNetwork ServerNetwork;

ClientNetwork *createClientNetwork(char *ipString, int port);
ServerNetwork *createServerNetwork(int port);
int connectToServer(ClientNetwork *pClientNet);
void destroyClientNetwork(ClientNetwork *pClientNet);
void messageBuffer(ServerNetwork *pServerNet);
void acceptClients(ServerNetwork *pServerNet);
void destroyServerNetwork(ServerNetwork *pServerNet);
int holdUntilConnected(ClientNetwork *pClientNet, int timeout);
void sendName(ClientNetwork *pClientNet,  char *name); 
int readFromServer(ClientNetwork *pClient, char *packet, int bufferSize);

// Add missing function declarations
void sendPacket(ClientNetwork *pClientNet, char *packet, int size);
bool isHost(ServerNetwork *pServerNet);
bool allPlayersReady(ServerNetwork *pServerNet);

#endif
