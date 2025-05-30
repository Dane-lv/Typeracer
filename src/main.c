#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>

struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
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


    pGame->isRunning = true;
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
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);

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
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    SDL_Quit();
}