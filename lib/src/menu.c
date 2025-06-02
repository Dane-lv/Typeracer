#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include "text.h"
#include "menu.h"

struct menu{

    SDL_Renderer *pRenderer;
    SDL_Window *pWindow;
    int window_width, window_height;
    TTF_Font *pFontButton;
    TTF_Font *pFontTitle;
    Text *pConnectText, *pHostServer, *pSettingsText, *pTitleText;
};

Menu *createMenu(SDL_Renderer *pRenderer, SDL_Window *pWindow, int WIN_H, int WIN_W){
    Menu *pMenu = malloc(sizeof(struct menu));
    if (!pMenu) return NULL;
    pMenu->window_width = WIN_W;
    pMenu->window_height = WIN_H;
    pMenu->pRenderer = pRenderer;
    pMenu->pWindow = pWindow;
    pMenu->pFontTitle = TTF_OpenFont("lib/resources/arial.ttf", 80);
    pMenu->pFontButton = TTF_OpenFont("lib/resources/arial.ttf", 50);
    if(!pMenu->pFontTitle || !pMenu->pFontButton){
        printf("Error menu text access: %s", SDL_GetError());
        destroyMenu(pMenu);
        return NULL;
    }
    pMenu->pTitleText = createText(pMenu->pRenderer, 255, 255, 255, pMenu->pFontTitle, "Skills Arena", pMenu->window_width/2+100, 100);
    if(!pMenu->pTitleText){
        destroyMenu(pMenu);
        return NULL;
    }
    pMenu->pConnectText = createText(pMenu->pRenderer, 200, 200, 200, pMenu->pFontButton, "CONNECT", pMenu->window_width/2+100, 250);
    if(!pMenu->pConnectText){
        destroyMenu(pMenu);
        return NULL;
    }
    pMenu->pHostServer = createText(pMenu->pRenderer, 200, 200, 200, pMenu->pFontButton, "HOST SERVER", pMenu->window_width/2+100, 350);
    if(!pMenu->pHostServer){
        destroyMenu(pMenu);
        return NULL;
    }
    pMenu->pSettingsText = createText(pMenu->pRenderer, 200, 200, 200, pMenu->pFontButton, "SETTINGS", pMenu->window_width/2+100, 450);
    if(!pMenu->pSettingsText){
        destroyMenu(pMenu);
        return NULL;
    }

    return pMenu;
}

void renderMenu(Menu *pMenu){
    drawText(pMenu->pTitleText);
    drawText(pMenu->pConnectText);
    drawText(pMenu->pHostServer);
    drawText(pMenu->pSettingsText);

}

void destroyMenu(Menu *pMenu){
    if (pMenu->pTitleText) destroyText(pMenu->pTitleText);
    if (pMenu->pConnectText) destroyText(pMenu->pConnectText);
    if (pMenu->pHostServer)    destroyText(pMenu->pHostServer);
    if (pMenu->pSettingsText)    destroyText(pMenu->pSettingsText);
    if (pMenu->pFontButton) TTF_CloseFont(pMenu->pFontButton);
    if (pMenu->pFontTitle)  TTF_CloseFont(pMenu->pFontTitle);
    free(pMenu);
}