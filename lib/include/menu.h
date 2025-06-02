#ifndef menu_h
#define menu_h

typedef struct menu Menu;

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Window *pWindow, int WIN_H, int WIN_W);
void destroyMenu(Menu *pMenu);
void renderMenu(Menu *pMenu);


#endif