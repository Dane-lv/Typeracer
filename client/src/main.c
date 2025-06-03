#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_net/SDL_net.h>
#include "text.h"
#include "stateAndData.h"
#include "menu.h"

struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    bool isRunning;
    ClientState state;
    Menu *pMenu;
    IpBar *pIpBar;
};
typedef struct game Game;

bool init(Game *pGame);
void close(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame);
void updateGame(Game *pGame);

int main(){
    Game g = {0};

    if(!init(&g)) return 1;
    run(&g);
    close(&g);

    return 0;
}

bool init(Game *pGame){
    if (!SDL_Init(SDL_INIT_VIDEO)){
        printf("Error SDL Init: %s\n", SDL_GetError());
        return false;
    }

    if (!TTF_Init()){
        printf("Error: %s\n",SDL_GetError());
        SDL_Quit();
        return false;
    }
    if (!NET_Init()){
        printf("Error Net init: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    pGame->pWindow = SDL_CreateWindow("Skills Arena Client", 800, 600, SDL_WINDOW_RESIZABLE);
    if(!pGame->pWindow){
        printf("Error intializing window: %s\n", SDL_GetError());
        close(pGame);
        return false;
    }   
    
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, NULL);
    if(!pGame->pRenderer){
        printf("Error initializing renderer: %s\n", SDL_GetError());
        close(pGame);
        return false;
    }
    pGame->pMenu = createMenu(pGame->pRenderer, pGame->pWindow, 800, 600);
    if(!pGame->pMenu){
        printf("Error menu init: %s\n", SDL_GetError());
        return false;
    }

    pGame->state = CLIENT_MENU;
    pGame->isRunning = true;
    return true;
}


void handleInput(Game *pGame){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_EVENT_QUIT){
            pGame->isRunning = false;
            return;
        }
        int menuOptionClicked, ipBarResult;
        switch(pGame->state){
            case CLIENT_MENU:
                menuOptionClicked = menuOptionsEvent(pGame->pMenu, &event);
                if(menuOptionClicked == 1){
                    pGame->pIpBar = createIpBar(pGame->pRenderer, pGame->pWindow, 800, 600);
                    if(!pGame->pIpBar){
                        printf("Error ipbar init: %s\n", SDL_GetError());
                        return;
                    }
                    else {
                        pGame->state = CLIENT_ENTER_IP;
                        SDL_StartTextInput(pGame->pWindow);
                        break; 
                    }
                }
                if(menuOptionClicked == 2){
                    // host server
                }
                if(menuOptionClicked == 3){
                    // settingswindow
                }
                break;
                
            case CLIENT_ENTER_IP:
                ipBarResult = IpBarHandle(pGame->pIpBar, &event);
                if(ipBarResult == 1){
                    //check if ip is right...
                    SDL_StopTextInput(pGame->pWindow);
                    destroyIpBar(pGame->pIpBar);
                    pGame->pIpBar = NULL;
                    pGame->state = CLIENT_MENU;
                    // TODO: game state -> lobby;
                }
                else if(ipBarResult == 2){
                    SDL_StopTextInput(pGame->pWindow);
                    destroyIpBar(pGame->pIpBar);
                    pGame->pIpBar = NULL;
                    pGame->state = CLIENT_MENU;
                }
                break;
                
        
            default: break;

        }  
    }
}

void renderGame(Game *pGame){
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);

    switch(pGame->state){
        case CLIENT_MENU:
            renderMenu(pGame->pMenu);
            break;
        case CLIENT_ENTER_IP:
            renderIpBar(pGame->pIpBar);
            break;
        default: break;
    }
    SDL_RenderPresent(pGame->pRenderer);

    
}

void updateGame(Game *pGame){


}

void run(Game *pGame){ //Eventual server/data handling here...?

    while(pGame->isRunning){
        handleInput(pGame);
        updateGame(pGame);
        renderGame(pGame);
      
    }

}

void close(Game *pGame){
    if(pGame->pMenu) destroyMenu(pGame->pMenu);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    TTF_Quit(); 
    NET_Quit();
    SDL_Quit();
}