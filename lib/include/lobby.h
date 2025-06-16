#ifndef lobby_h
#define lobby_h

#include "stateAndData.h"

typedef struct lobby Lobby;



void renderLobby(Lobby *pLobby);
Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height);
void destroyLobby(Lobby *pLobby);
int nameInputHandle(Lobby *pLobby, SDL_Event *event);
char *getName(Lobby *pLobby);
ClientData *lobby_getPlayersLocal(Lobby *pLobby);



#endif