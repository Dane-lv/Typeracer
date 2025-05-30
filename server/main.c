#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "text.h"
#include "stateAndData.h"


struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    TTF_Font *pFont, *pWaitingText;
    GameState state;
    bool isRunning;
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
    if (TTF_Init()! = 0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("Skills Arena", 800, 600, SDL_WINDOW_RESIZABLE);
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

    pGame->pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    if(!pGame->pFont){
        printf("Error font access: %s\n",TTF_GetError());
        close(pGame);
        return 0;
    }
    pGame->pWaitingText = createText(pGame->pRenderer,238,168,65,pGame->pScoreFont,"Waiting for clients",WINDOW_WIDTH/2,WINDOW_HEIGHT/2+100);
    if(!pGame->pStartText){
        printf("Error waiting text: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }

    pGame->isRunning = true;
    pGame->state = LOBBY;
    return true;
}


void handleInput(Game *pGame){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_EVENT_QUIT:
                pGame->isRunning = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if(event.key.scancode == SDL_SCANCODE_ESCAPE) {pGame->isRunning = false;} break;

            default: 
                break;    
            }    
    }
}

void renderGame(Game *pGame){
    switch(pGame->state){
        case LOBBY:
            drawText(pGame->pStartText);
            SDL_RenderPresent(pGame->pRenderer);
    }

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
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    SDL_Quit();
}