#ifndef network_h
#define network_h

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




#endif
