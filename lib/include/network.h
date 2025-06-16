#ifndef network_h
#define network_h

typedef struct server Server;
typedef struct client Client;
typedef struct lobby Lobby;




Server *createServer();
void destroyServer(Server *pSrv);
void acceptClients(Server *pSrv);

Client *createClient(char *ipString, int port);
void destroyClient(Client *pCli);
void sendPlayerName(Client *pCli, char *playerName);
void readFromClients(Server *pSrv);
void writeToClients(Server *pSrv);
void readFromServer(Client *pCli, Lobby *pLobby);


#endif