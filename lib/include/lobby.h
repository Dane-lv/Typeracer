#ifndef lobby_h
#define lobby_h

#include "stateAndData.h"



typedef struct lobby Lobby;




void renderLobby(Lobby *pLobby);
Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height, bool hostCheck);
void destroyLobby(Lobby *pLobby);
int nameInputHandle(Lobby *pLobby, SDL_Event *event);
char *getName(Lobby *pLobby);
LobbyData *getLobbyLocal(Lobby *pLobby);
bool isStillTyping(Lobby *pLobby);
void setLobbyChanged(Lobby *pLobby, bool changed);
void renderNamesAndStatus(Lobby *pLobby);
void updateLobby(Lobby *pLobby);
int lobbyEventHandle(Lobby *pLobby, SDL_Event *event);
bool getReadyStatus(Lobby *pLobby);
bool arePlayersReady(Lobby *pLobby);
bool hostCheck(Lobby *pLobby);



#endif