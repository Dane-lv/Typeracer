#ifndef state_and_data_h
#define state_and_data_h

#define MAXNAME 12

enum gameState{MENU, ENTER_IP, LOBBY, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum messageType{MSG_NAME = 1};
typedef enum messageType MessageType;

struct playerData{
    char name[MAXNAME+1];
    int score;
    bool isReady;
};
typedef struct playerData PlayerData;


#endif