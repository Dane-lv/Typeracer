#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "text.h"

struct menu{

    SDL_Renderer *pRenderer;
    SDL_Window *pWindow;
    int window_width, window_height;
    TTF_Font *pFontButton;
    TTF_Font *pFontTitle;
    Text *pConnectText, *pHostServer, *pSettingsText, *pTitleText;
};

struct ipBar{
    SDL_Renderer *pRenderer;
    SDL_Window *pWindow;
    TTF_Font *pInputFont, *pPromptFont;
    int window_width, window_height;
    Text *pInputText, *pPromptText, *pStatusText;
    char buffer[64];
    int length;
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

IpBar *createIpBar(SDL_Renderer *pRenderer, SDL_Window *pWindow, int x, int y){
    IpBar *pIpBar = malloc(sizeof(struct ipBar));
    if(!pIpBar) return NULL;
    pIpBar->pRenderer = pRenderer;
    pIpBar->pWindow = pWindow;
    pIpBar->pPromptFont = TTF_OpenFont("lib/resources/bodoni.ttf", 35);
    pIpBar->pInputFont = TTF_OpenFont("lib/resources/arial.ttf", 25);
    if (!pIpBar->pPromptFont || !pIpBar->pInputFont){
        printf("Prompt text access fail: %s\n", SDL_GetError());
        destroyIpBar(pIpBar);
        return NULL;
    }
    pIpBar->pPromptText = createText(pIpBar->pRenderer, 255, 255, 255, pIpBar->pPromptFont, "Enter server IP:", x/2, y/2-100);
    if (!pIpBar->pPromptText){
        printf("Prompt text fail: %s\n", SDL_GetError());
        destroyIpBar(pIpBar);
        return NULL;
    }
    pIpBar->pInputText = NULL;
    pIpBar->window_width = x;
    pIpBar->window_height = y;
    pIpBar->buffer[0] = '\0';
    pIpBar->length = 0;
    return pIpBar;
}

void renderIpBar(IpBar *pIpBar){
    drawText(pIpBar->pPromptText);
    if(pIpBar->pInputText){
        drawText(pIpBar->pInputText);
    }
    if(pIpBar->pStatusText) drawText(pIpBar->pStatusText);
}

void renderMenu(Menu *pMenu){
    drawText(pMenu->pTitleText);
    drawText(pMenu->pConnectText);
    drawText(pMenu->pHostServer);
    drawText(pMenu->pSettingsText);

}

int IpBarHandle(IpBar *pIpBar, SDL_Event *event){
    switch (event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pIpBar->length + strlen(event->text.text)){
                strcat(pIpBar->buffer, event->text.text);
                pIpBar->length += strlen(event->text.text);
                if(pIpBar->pInputText) destroyText(pIpBar->pInputText);
                pIpBar->pInputText = createText(pIpBar->pRenderer, 200, 198, 145, pIpBar->pInputFont, pIpBar->buffer,
                pIpBar->window_width/2, pIpBar->window_height/2-50);
                return 0;
            }
            break;
        
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pIpBar->length > 0){
                    pIpBar->buffer[--pIpBar->length] = '\0';
                    if(pIpBar->pInputText) destroyText(pIpBar->pInputText);
                    pIpBar->pInputText = createText(pIpBar->pRenderer, 200, 198, 145, pIpBar->pInputFont, pIpBar->buffer,
                    pIpBar->window_width/2, pIpBar->window_height/2-50);
                    return 0;
                }
            }
            else if(event->key.scancode == SDL_SCANCODE_RETURN || event->key.scancode == SDL_SCANCODE_KP_ENTER){
                if(pIpBar->length > 0){ //user pressend enter with nonempty buffer
                    return 1;
                }
                else {
                    return 0;} //buffer is empty
            }  
            else if(event->key.scancode == SDL_SCANCODE_ESCAPE){
                return 2;
            }
            break;
        
        default: break; 
        }

        return 0;
}

int menuOptionsEvent(Menu *pMenu, SDL_Event *event){
    switch (event->type){
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(event->button.button == SDL_BUTTON_LEFT){
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                SDL_FRect connectRect = getTextRect(pMenu->pConnectText);
                if(mouseX >= connectRect.x && mouseX <= (connectRect.x + connectRect.w) && mouseY >= connectRect.y && mouseY <= (connectRect.y + connectRect.h)){
                    return 1;
                }
                break;
            }
        default: break;
    }

    return 0;
}

void showIpBarStatus(IpBar *pIpBar, char *msg, int r, int g, int b) {
    if (pIpBar->pStatusText) destroyText(pIpBar->pStatusText);
    pIpBar->pStatusText = createText(pIpBar->pRenderer, r, g, b, pIpBar->pInputFont, (char *)msg, pIpBar->window_width / 2,
    pIpBar->window_height/2+50);
}


char *getIpAdress(IpBar *pIpBar){
    return pIpBar->buffer;
}

void destroyIpBar(IpBar *pIpBar){
    
    if (pIpBar->pStatusText) destroyText(pIpBar->pStatusText);
    if (pIpBar->pPromptText) destroyText(pIpBar->pPromptText);
    if (pIpBar->pPromptFont) TTF_CloseFont(pIpBar->pPromptFont);
    if (pIpBar->pInputFont) TTF_CloseFont(pIpBar->pInputFont);

    free(pIpBar);
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