#ifndef netudp_h
#define netudp_h

typedef struct serverUDP ServerUDP;
typedef struct clientUDP ClientUDP;


ServerUDP *createUDPServer();
ClientUDP *createUDPClient(char *ipString);
void destroyUDPServer(ServerUDP *pSrvUDP);
void destroyUDPClient(ClientUDP *pCliUDP);

#endif