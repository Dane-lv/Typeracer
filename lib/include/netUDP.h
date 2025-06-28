#ifndef netudp_h
#define netudp_h

typedef struct serverUDP ServerUDP;
typedef struct clientUDP ClientUDP;


ServerUDP *createUDPServer();
ClientUDP *createUDPClient(char *ipString, int index);
void destroyUDPServer(ServerUDP *pSrvUDP);
void destroyUDPClient(ClientUDP *pCliUDP);

void sendClientInfoToUDP(ClientUDP *pCliUDP);
void readFromClientsUDP(ServerUDP *pSrvUDP);



#endif