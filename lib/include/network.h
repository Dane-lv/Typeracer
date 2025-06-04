#ifndef network_h
#define network_h

typedef struct network Network;

Network *createNetwork(char *ipString, int port);
int connectToServer(Network *pNetwork);
void destroyNetwork(Network *pNetwork);


#endif
