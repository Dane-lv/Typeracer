#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include "text.h"

struct text{
    SDL_FRect rect;
    SDL_Texture *pTexture;
    SDL_Renderer *pRenderer;
};

Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString, int x, int y) {
    Text *pText = malloc(sizeof(struct text));
    if (!pText) return NULL;

    pText->pRenderer = pRenderer;

    SDL_Color color = { r, g, b, 255 };

    SDL_Surface *pSurface = TTF_RenderText_Solid(pFont, pString, strlen(pString),color);
    if (!pSurface) {
        printf("Error rendering text surface: %s\n", SDL_GetError());
        free(pText);
        return NULL;
    }

    pText->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_DestroySurface(pSurface);

    if (!pText->pTexture) {
        printf("Error creating text texture: %s\n", SDL_GetError());
        free(pText);
        return NULL;
    }

    float w, h;
    SDL_GetTextureSize(pText->pTexture, &w, &h);
    pText->rect.w = w;
    pText->rect.h = h;
    pText->rect.x = x - w / 2.0f;
    pText->rect.y = y - h / 2.0f;

    return pText;
}


void drawText(Text *pText){
    SDL_RenderTexture(pText->pRenderer,pText->pTexture, NULL, &pText->rect);
}

void destroyText(Text *pText){
    SDL_DestroyTexture(pText->pTexture);
    free(pText);
}