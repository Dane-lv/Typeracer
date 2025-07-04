#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "text.h"
#include "main.h"

#define MAXIPLEN 16

struct menu
{
    int window_width, window_heigth;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_FRect titleRect, connectRect, hostGameRect, settingsRect;
    TTF_Font *pMenuFontTitle, *pMenuFontButtons;
    Text *pTitleText, *pButtonTextConnect, *pButtonTextHostGame, *pButtonTextSettings;
};

struct ipBar
{
    int window_width, window_height;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    TTF_Font *pFont;
    char ipString[MAXIPLEN];
    int length;
    Text *pPromptText, *pInputText;
};

Menu *createMenu(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int heigth){
    Menu *pMenu = malloc(sizeof(struct menu));
    if(!pMenu){
        SDL_OutOfMemory();
        return NULL;
    }
    pMenu->window_width = width;
    pMenu->window_heigth = heigth;
    pMenu->pWindow = pWindow;
    pMenu->pRenderer = pRenderer;
    /* TEXT INIT */
    pMenu->pMenuFontTitle = TTF_OpenFont(FONT_PATH_MENU_TITLE,FONT_SIZE_MENU_TITLE);
    if(!pMenu->pMenuFontTitle){ printf("%s\n", SDL_GetError());return NULL;}
    pMenu->pMenuFontButtons = TTF_OpenFont(FONT_PATH_MENU_BUTTONS, FONT_SIZE_MENU_BUTTONS);
    if(!pMenu->pMenuFontButtons){printf("%s\n", SDL_GetError());  return NULL;}
    pMenu->pTitleText = createText(pMenu->pRenderer, 255, 255 ,255, pMenu->pMenuFontTitle, "TYPERACER", width/2, heigth/5, true);
    if(!pMenu->pTitleText) {printf("%s\n", SDL_GetError()); return NULL;}
    pMenu->pButtonTextConnect = createText(pMenu->pRenderer, 255, 255 ,255, pMenu->pMenuFontButtons, "CONNECT", width/2, heigth/3, true);
    pMenu->pButtonTextHostGame = createText(pMenu->pRenderer, 255, 255 ,255, pMenu->pMenuFontButtons, "HOST GAME", width/2, heigth/3 + 120, true);
    pMenu->pButtonTextSettings = createText(pMenu->pRenderer, 255, 255 ,255, pMenu->pMenuFontButtons, "SETTINGS", width/2, heigth/3 + 240, true);
    if(!pMenu->pButtonTextConnect|| !pMenu->pButtonTextHostGame || !pMenu->pButtonTextSettings){printf("%s\n", SDL_GetError()); return NULL;}
    pMenu->titleRect = getTextRect(pMenu->pTitleText);
    pMenu->connectRect = getTextRect(pMenu->pButtonTextConnect);
    pMenu->hostGameRect = getTextRect(pMenu->pButtonTextHostGame);
    pMenu->settingsRect = getTextRect(pMenu->pButtonTextSettings);
    /*         */
    return pMenu;
}

IpBar *createIpBar(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height){
    IpBar *pIpBar = malloc(sizeof(struct ipBar));
    pIpBar->pWindow = pWindow;
    pIpBar->pRenderer = pRenderer;
    pIpBar->window_height = height;
    pIpBar->window_width = width;
    pIpBar->pFont = TTF_OpenFont(FONT_PATH_MENU_BUTTONS, FONT_SIZE_MENU_BUTTONS);
    if(!pIpBar->pFont) {printf("%s\n", SDL_GetError()); return NULL;}
    pIpBar->pPromptText = createText(pIpBar->pRenderer, 255, 255 ,255, pIpBar->pFont, "Enter IP:", width/2, 175, true);
    pIpBar->pInputText = NULL;
    pIpBar->ipString[0] = '\0';
    pIpBar->length = 0;

    return pIpBar;
}

int ipAddressInputHandle(IpBar *pIpBar, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pIpBar->length + strlen(event->text.text) < MAXIPLEN){
                strcat(pIpBar->ipString, event->text.text);
                pIpBar->length = strlen(pIpBar->ipString);
                if(pIpBar->pInputText) destroyText(pIpBar->pInputText);
                pIpBar->pInputText = createText(pIpBar->pRenderer, 202, 202 ,0, pIpBar->pFont, pIpBar->ipString, pIpBar->window_width/2, 240, true);
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pIpBar->length > 0){
                    pIpBar->length--;
                    pIpBar->ipString[pIpBar->length] = '\0';
                    if(pIpBar->pInputText) destroyText(pIpBar->pInputText);
                    pIpBar->pInputText = createText(pIpBar->pRenderer, 202, 202 ,0, pIpBar->pFont, pIpBar->ipString, pIpBar->window_width/2, 240, true);
                }
            }
            else if(event->key.scancode == SDL_SCANCODE_RETURN || event->key.scancode == SDL_SCANCODE_KP_ENTER){
                if(pIpBar->length > 0) {return 1;}
                else {return 2;}
            }
            else if(event->key.scancode == SDL_SCANCODE_ESCAPE){
                return 3;
            }
                break;
            
        default: 
            break;
    }
    return 0;
}



int menuOptionsEvent(Menu *pMenu, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(event->button.button == SDL_BUTTON_LEFT){
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY); // is it within bounds
                if(mouseX >= pMenu->connectRect.x && mouseX <= (pMenu->connectRect.x + pMenu->connectRect.w)
                    && mouseY >= pMenu->connectRect.y && mouseY <=(pMenu->connectRect.y + pMenu->connectRect.h)){
                        return 1;
                }
                if(mouseX >= pMenu->hostGameRect.x && mouseX <= (pMenu->hostGameRect.x + pMenu->hostGameRect.w)
                    && mouseY >= pMenu->hostGameRect.y && mouseY <=(pMenu->hostGameRect.y + pMenu->hostGameRect.h)){
                        return 2;
                }
                
            }
    }
    return 0;
}

char *getIp(IpBar *pIpBar){
    return pIpBar->ipString;
}

void renderIpBar(IpBar *pIpBar){
    drawText(pIpBar->pPromptText);
    if(pIpBar->pInputText) drawText(pIpBar->pInputText);
}

void renderMenu(Menu *pMenu){
    drawText(pMenu->pTitleText);
    drawText(pMenu->pButtonTextConnect);
    drawText(pMenu->pButtonTextHostGame);
    drawText(pMenu->pButtonTextSettings);
}

void destroyIpBar(IpBar *pIpBar){
    if(pIpBar->pFont) TTF_CloseFont(pIpBar->pFont);
    if(pIpBar->pPromptText) destroyText(pIpBar->pPromptText);
    if(pIpBar->pInputText) destroyText(pIpBar->pInputText);
    free(pIpBar);
}

void destroyMenu(Menu *pMenu){
    if(pMenu->pMenuFontTitle) TTF_CloseFont(pMenu->pMenuFontTitle);
    if(pMenu->pMenuFontButtons) TTF_CloseFont(pMenu->pMenuFontButtons);
    if(pMenu->pTitleText) destroyText(pMenu->pTitleText);
    if(pMenu->pButtonTextConnect) destroyText(pMenu->pButtonTextConnect);
    if(pMenu->pButtonTextHostGame) destroyText(pMenu->pButtonTextHostGame);
    if(pMenu->pButtonTextSettings) destroyText(pMenu->pButtonTextSettings);
    free(pMenu);
}