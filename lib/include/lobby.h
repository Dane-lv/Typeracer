#ifndef lobby_h
#define lobby_h
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct lobby Lobby;

Lobby *createLobby(SDL_Renderer *pRenderer, SDL_Window *pWindow, int width, int height, bool isHost);
int lobbyNameInputHandle(Lobby *pLobby, SDL_Event *event);
void destroyLobby(Lobby *pLobby);
bool isDoneTypingName(Lobby *pLobby);
void renderNameInput(Lobby *pLobby);
char *returnName(Lobby *pLobby);
void lobbyAddPlayer(Lobby *pLobby, char *name);
void renderLobby(Lobby *pLobby);

// Add missing function declarations
void lobbySetReady(Lobby *pLobby, int idx, bool ready);
bool lobbyAllPlayersReady(Lobby *pLobby);
bool lobbyGetReady(Lobby *pLobby, int idx);

// Add new name-based ready functions
bool lobbyIsPlayerReady(Lobby *pLobby, char *playerName);
void lobbySetPlayerReady(Lobby *pLobby, char *playerName, bool ready);

#endif