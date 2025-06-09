#ifndef state_and_data_h
#define state_and_data_h

#define MAXNAME 12
#define MAXPLAYERS 4
#define BUFFERSIZE 512

enum gameState{MENU, ENTER_IP, LOBBY, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum messageType{MSG_NAME = 1, MSG_READY = 2};
typedef enum messageType MessageType;




#endif