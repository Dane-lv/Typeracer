#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include "text.h"
#include "main.h"

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
    SDL_Surface *pSurface = TTF_RenderText_Blended(pFont, pString, 0,color);
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

    int input_x = (int)(WINDOW_WIDTH / 6.3f); // For the input text's left allignment
    if (x == input_x) {               
        pText->rect.w = w;
        pText->rect.h = h;
        pText->rect.x = (float)x;             
        pText->rect.y = (float)y - h / 2.0f;
    }
    else{
        pText->rect.w = w;
        pText->rect.h = h;
        pText->rect.x = x - w / 2.0f;
        pText->rect.y = y - h / 2.0f;
    }
    return pText;
}

SDL_FRect getTextRect(Text *pText){ // read-only
    return pText->rect;
}

void drawText(Text *pText){
    SDL_RenderTexture(pText->pRenderer,pText->pTexture, NULL, &pText->rect);
}


SDL_FRect *getRect(Text *pText){ // can modify
    return &pText->rect;
}

void destroyText(Text *pText){
    SDL_DestroyTexture(pText->pTexture);
    free(pText);
}