#ifndef state_and_data_h
#define state_and_data_h
#include "main.h"

enum gameState {MENU, IP_INPUT, LOBBY, ONGOING, ROUND_OVER};
typedef enum gameState GameState;

struct clientData { // FOR LOBBY
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

struct playerData{
    char playerName[MAXNAME];
    // WPM
    // OTHER MEMBERS
};
typedef struct playerData PlayerData;

struct gameCoreData{
    PlayerData players[MAXCLIENTS];
    int nrOfPlayers;
};
typedef struct gameCoreData GameCoreData;

struct textData{
    char text[MAX_TEXT_LEN];
    int chosenText;

};
typedef struct textData TextData;


enum messageType{MSG_NAME = 1, MSG_PLAYER_INDEX= 2, MSG_READY = 3, MSG_CLIENT_INFO = 4, MSG_START_GAME = 5, MSG_LOBBY = 100};
typedef enum messageType MessageType;





#endif