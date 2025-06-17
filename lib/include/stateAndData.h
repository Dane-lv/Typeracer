#ifndef state_and_data_h
#define state_and_data_h
#include "main.h"

enum gameState {MENU, IP_INPUT, LOBBY, ONGOING, ROUND_OVER};
typedef enum gameState GameState;

struct clientData {
    char playerName[MAXNAME];
    bool isReady;
    bool isHost;
};
typedef struct clientData ClientData;

struct lobbyData{
    ClientData players[MAXCLIENTS];
    int nrOfPlayers;
};
typedef struct lobbyData LobbyData;

enum messageType{MSG_NAME = 1, MSG_READY = 2, MSG_START_GAME = 3, MSG_LOBBY = 100};
typedef enum messageType MessageType;





#endif