#ifndef state_and_data_h
#define state_and_data_h

#define MAXNAME 12
#define MAXPLAYERS 4
#define BUFFERSIZE 32 // In tcp, only used for sending name and ready status.

enum gameState{MENU, ENTER_IP, LOBBY, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum messageType{MSG_NAME = 1, MSG_READY = 2, MSG_START_GAME};
typedef enum messageType MessageType;

struct clientData{
    char playerName[MAXNAME];
    bool isReady;
};
typedef struct clientData ClientData;

typedef struct {
    Uint8 type;
    Uint8 playerIndex;
    char name[MAXNAME-2];
} NamePacket;

typedef struct {
    Uint8 type;
    Uint8 playerIndex;
} ReadyPacket;

typedef struct{
    Uint8 type;
} StartGamePacket;


#endif