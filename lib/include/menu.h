#ifndef menu_h
#define menu_h
//MENU AND IP BAR HEADERS
typedef struct menu Menu;
typedef struct ipBar IpBar;

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Window *pWindow, int WIN_H, int WIN_W);
void destroyMenu(Menu *pMenu);
void destroyIpBar(IpBar *pIpBar);
void renderMenu(Menu *pMenu);
int menuOptionsEvent(Menu *pMenu, SDL_Event *event);
void renderIpBar(IpBar *pIpBar);
IpBar *createIpBar(SDL_Renderer *pRenderer, SDL_Window *pWindow, int x, int y);
int IpBarHandle(IpBar *pIpBar, SDL_Event *event);
char *getIpAdress(IpBar *pIpBar);
void showIpBarStatus(IpBar *pIpBar, char *msg, int r, int g, int b);


#endif