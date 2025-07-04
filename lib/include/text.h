#ifndef text_h
#define text_h

#include <stdbool.h>

typedef struct text Text;

Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString, int x, int y, bool centered);
void drawText(Text *pText);
void destroyText(Text *pText);
SDL_FRect *getRect(Text *pText);
SDL_FRect getTextRect(Text *pText);


#endif