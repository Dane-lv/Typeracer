#ifndef lobby_h
#define lobby_h
#include <SDL3/SDL.h>

typedef struct lobby Lobby;

Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height);
int lobbyNameInputHandle(Lobby *pLobby, SDL_Event *event);
void destroyLobby(Lobby *pLobby);
bool isDoneTypingName(Lobby *pLobby);
void sendName(ClientNetwork *pClientNet, Lobby *pLobby);
void renderNameInput(Lobby *pLobby);


#endif