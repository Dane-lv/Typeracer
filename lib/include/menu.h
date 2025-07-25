#ifndef menu_h
#define menu_h

typedef struct audio Audio;
typedef struct menu Menu;
typedef struct ipBar IpBar;


Menu *createMenu(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int heigth, Audio *pAudio);
void renderMenu(Menu *pMenu);
void destroyMenu(Menu *pMenu);
int menuOptionsEvent(Menu *pMenu, SDL_Event *event, Audio *pAudio);
IpBar *createIpBar(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int heigth);
void renderIpBar(IpBar *pIpBar);
int ipAddressInputHandle(IpBar *pIpBar, SDL_Event *event);
void destroyIpBar(IpBar *pIpBar);
char *getIp(IpBar *pIpBar);

#endif