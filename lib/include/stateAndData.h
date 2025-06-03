#ifndef state_and_data_h
#define state_and_data_h

#define BUFFERSIZE 512


enum clientState{CLIENT_MENU, CLIENT_ENTER_IP, CLIENT_LOBBY, CLIENT_JOIN, CLIENT_ONGOING, CLIENT_GAME_OVER};
typedef enum clientState ClientState;

enum gameState{SERVER_LOBBY, SERVER_ONGOING, SERVER_GAME_OVER};
typedef enum gameState GameState;



#endif