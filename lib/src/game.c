#include "main.h"
#include <time.h>
#include <math.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include "audio.h"
#include <stdlib.h>
#include "text.h"
#include "netTCP.h"
#include "netUDP.h"
#include "stateAndData.h"
#include "game.h"

struct gameCore{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    TTF_Font *pNamesFont;
    TTF_Font *pTextFont;
    TTF_Font *pNumbersFont;
    TTF_Font *pPlacementFont;
    TTF_Font *pWpmTextFont;
    Text *pNames[MAXCLIENTS];
    Text *pTextAsWords[MAXTEXTWORD];
    Text *pInputText;
    Text *pCountdownText[6];
    Text *pPlacements[MAXCLIENTS];
    SDL_Texture *pCars[MAXCLIENTS];
    SDL_FRect cars[MAXCLIENTS];
    int window_width, window_height;
    GameCoreData gData_local;
    TextData tData;
    SDL_FRect inputBox;
    SDL_FRect highlightInputBox;
    SDL_FRect blinkingCursor;
    SDL_FRect lowerBoxRect;
    SDL_FRect highlightLowerBox;
    SDL_FRect highlightWindow;
    char inputString[MAXINPSTR];
    int inpStrLen;
    float startTime;
    bool startedTyping;
    char wpm[4];
    bool isGameChanged;
    Text *pWPM[MAXCLIENTS];
    Text *pWPMText[MAXCLIENTS];
    bool isWrongLetterTyped;
    bool countdownFinished;
    bool playSound;
    bool isTextFinished;
    int startTimeForCounter;

};


GameCore *createGameCore(SDL_Window *pWindow, SDL_Renderer *pRenderer, int width, int height, int textToLoad, Audio *pAudio){
    GameCore *pCore = malloc(sizeof(struct gameCore));

    destroyBgMusic(pAudio); // stop bg music
    pCore->playSound = true;


    pCore->pWindow = pWindow;
    pCore->pRenderer = pRenderer;
    pCore->window_width = width;
    pCore->window_height = height;
    pCore->pNamesFont = TTF_OpenFont(FONT_PATH_CORE_NAMES, FONT_SIZE_CORE_NAMES);
    pCore->pTextFont = TTF_OpenFont(FONT_PATH_GAME, FONT_SIZE_GAME);
    pCore->pNumbersFont = TTF_OpenFont(FONT_PATH_GAME_NUMBERS, FONT_SIZE_GAME_NUMBERS);
    pCore->pPlacementFont = TTF_OpenFont(FONT_PATH_CORE_NAMES, FONT_SIZE_PLACEMENT);
    pCore->pWpmTextFont = TTF_OpenFont(FONT_PATH_CORE_NAMES, FONT_SIZE_WPMTEXT);
    SDL_memset(pCore->pPlacements, 0, sizeof(pCore->pPlacements));
    SDL_memset(pCore->pCountdownText, 0, sizeof(pCore->pCountdownText));
    SDL_memset(pCore->pNames, 0, sizeof(pCore->pNames));
    SDL_memset(pCore->pCars, 0, sizeof(pCore->pCars));
    SDL_memset(&pCore->gData_local, 0, sizeof(GameCoreData));
    SDL_memset(pCore->pTextAsWords, 0, sizeof(pCore->pTextAsWords));
    SDL_memset(pCore->pWPM,0,sizeof(pCore->pWPM));
    SDL_memset(pCore->pWPMText,0,sizeof(pCore->pWPMText));
    pCore->pInputText = NULL;
    SDL_memset(&pCore->tData, 0, sizeof(TextData));
    for(int i = 0; i < MAXCLIENTS; i++){
        if(i == 0) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car0.png");  SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 1) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car1.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 2) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car2.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
        if(i == 3) {pCore->pCars[i] = IMG_LoadTexture(pCore->pRenderer, "lib/resources/car3.png"); SDL_SetTextureBlendMode(pCore->pCars[i], SDL_BLENDMODE_BLEND);}
    }
    for(int i = 0; i < MAXCLIENTS; i++){
        pCore->cars[i] = (SDL_FRect){380, 103 + i * 85, 62, 62}; // CARS INITIAL POS
    }
    if(!readFromFile(pCore, textToLoad)) {printf("Error reading file %s: \n", SDL_GetError()); destroyGameCore(pCore); return NULL;}
    pCore->tData.currentWordIndex = 0;
    pCore->inputBox.x = pCore->window_width/6.6+15;
    pCore->inputBox.y = pCore->window_height / 1.5 + 140;
    pCore->inputBox.w = pCore->window_width/1.5;
    pCore->inputBox.h = 50;

    pCore->highlightInputBox.x = pCore->window_width/6.6+13;
    pCore->highlightInputBox.y = pCore->window_height / 1.5 + 137;
    pCore->highlightInputBox.w = pCore->window_width/1.5+3;
    pCore->highlightInputBox.h = 54;

    pCore->lowerBoxRect.x = 280;
    pCore->lowerBoxRect.y = 510;
    pCore->lowerBoxRect.w = 1425;
    pCore->lowerBoxRect.h = 500;

    pCore->highlightLowerBox.x = 278;
    pCore->highlightLowerBox.y = 508;
    pCore->highlightLowerBox.w = 1428;
    pCore->highlightLowerBox.h = 504;

    pCore->highlightWindow.x = 235;
    pCore->highlightWindow.y = 88;
    pCore->highlightWindow.w = WINDOW_WIDTH-469;
    pCore->highlightWindow.h = WINDOW_HEIGHT-138;

    pCore->blinkingCursor.w = 2;
    pCore->blinkingCursor.h = 35;

    SDL_memset(pCore->inputString, 0, sizeof(pCore->inputString));
    pCore->inpStrLen = 0;

    parseText(pCore);
    createTextAsWords(pCore);
    updateCursorPosition(pCore); // This now directly sets the cursor position

    pCore->startedTyping = false;
    pCore->isGameChanged = false;
    SDL_memset(pCore->wpm, 0, sizeof(pCore->wpm));
    pCore->isWrongLetterTyped = false;
    pCore->isTextFinished = false;

    pCore->countdownFinished = false;


    pCore->pCountdownText[0] = createText(pCore->pRenderer, 255,0,0, pCore->pNumbersFont,"5...", pCore->window_width/2,pCore->window_height/2-100, true);
    pCore->pCountdownText[1] = createText(pCore->pRenderer, 204,  51,   0, pCore->pNumbersFont,"4..", pCore->window_width/2,pCore->window_height/2-100, true);
    pCore->pCountdownText[2] = createText(pCore->pRenderer, 153, 102,   0, pCore->pNumbersFont,"3...", pCore->window_width/2,pCore->window_height/2-100, true);
    pCore->pCountdownText[3] = createText(pCore->pRenderer, 102, 153,   0, pCore->pNumbersFont,"2..", pCore->window_width/2,pCore->window_height/2-100, true);
    pCore->pCountdownText[4] = createText(pCore->pRenderer,  51, 204,   0, pCore->pNumbersFont,"1.", pCore->window_width/2,pCore->window_height/2-100, true);
    pCore->pCountdownText[5] = createText(pCore->pRenderer, 0, 255,   0, pCore->pNumbersFont,"GO!", pCore->window_width/2,pCore->window_height/2-100, true);
    pCore->startTimeForCounter = SDL_GetTicks()/1000;





    return pCore;
}

void drawLines(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        float x1 = 380; // Car's texture initial position
        float x2 = pCore->window_width - 440; // end position
        float y1 = 159 + i * 85;
        float y2 = 159 + i * 85;
        int line_width = 10;
        int padding = 15;
        SDL_SetRenderDrawColor(pCore->pRenderer, 137, 137, 137,230);

        while(x1 < x2){
            SDL_RenderLine(pCore->pRenderer, x1, y1, x1+line_width, y2);
            x1 += line_width + padding;
        }
        

    }
    
}

void parseText(GameCore *pCore){ // split text into words
    pCore->tData.nrOfWords = 0;
    char *token = strtok(pCore->tData.text, " ");
    while(token && pCore->tData.nrOfWords < 100){
        pCore->tData.words[pCore->tData.nrOfWords++] = token;
        token = strtok(NULL, " ");
    }
}

void updateCursorPosition(GameCore *pCore){
    if(pCore->tData.currentWordIndex >= pCore->tData.nrOfWords) return; // no more words to type
    Text *currentWordText = pCore->pTextAsWords[pCore->tData.currentWordIndex];
    if(!currentWordText) {printf("Error current word text %s: \n", SDL_GetError()); return;}
    SDL_FRect *wordRect = getRect(currentWordText);
    
    int letterWidth = 0;
    if(pCore->inpStrLen > 0){
        // Get the current target word
        char *targetWord = pCore->tData.words[pCore->tData.currentWordIndex];
        if(pCore->inpStrLen <= (int)strlen(targetWord)){
            char targetSubstring[MAXINPSTR];
            strncpy(targetSubstring, targetWord, pCore->inpStrLen);
            targetSubstring[pCore->inpStrLen] = '\0';
            
            TTF_GetStringSize(pCore->pTextFont, targetSubstring, 0, &letterWidth, NULL);
        }
    }
    pCore->blinkingCursor.x = wordRect->x + letterWidth; // Position cursor at the beginning of the letter that is to be typed
    pCore->blinkingCursor.y= wordRect->y + 4; 

}

void calculateWPM(GameCore *pCore){
        unsigned int currentTime;
        currentTime = SDL_GetTicks();
        float currentTimeInSeconds= currentTime / 1000;

        printf("%f\n", currentTimeInSeconds);
        float timeCompletion = currentTimeInSeconds - pCore->startTime;
        int wpmNum = pCore->tData.currentWordIndex / (timeCompletion / 60);
        if(wpmNum > 400) return; 
        sprintf(pCore->wpm, "%d", wpmNum);

}

void updateCars(GameCore *pCore){
    int start_x = 380;
    int finish_x = pCore->window_width - 500;
    int total_distance = finish_x - start_x;
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        float procent_until_finish = (float)pCore->gData_local.players[i].playersCurrentWordIndex / pCore->tData.nrOfWords;
        pCore->cars[i].x  = start_x + (int)(procent_until_finish * total_distance);
    }      
    
}

int getCurrentWordIndex(GameCore *pCore){
    return pCore->tData.currentWordIndex;
}


void createTextAsWords(GameCore *pCore){
    int startX = pCore->window_width/6.3;
    int x = startX;
    int y = pCore->window_height/1.5 - 150;
    int lineHeight, spaceW;
    TTF_GetStringSize(pCore->pTextFont, " ", 0, &spaceW, &lineHeight); // calculate size of space
    for(int i = 0; i < pCore->tData.nrOfWords; i++){
        char *word = pCore->tData.words[i];
        if(word == NULL) continue;
        int wordW, wordH;           // Calculate how wide this word will be
                                    // Check if word would exceed right boundary
                                    // Right boundary = leftMargin + 70% of screen width
        TTF_GetStringSize(pCore->pTextFont, word, 0, &wordW, &wordH);
        if(x + wordW > (pCore->window_width/6.3 + pCore->window_width * 0.7)){
            x = startX;
            y += lineHeight;
        }
        pCore->pTextAsWords[i] = createText(pCore->pRenderer,255,255,255, pCore->pTextFont, word, x, y, false);
        x += wordW + spaceW;
    }
}

int gameCoreInputHandle(GameCore *pCore, SDL_Event *event){
    switch(event->type){
        case SDL_EVENT_TEXT_INPUT:
            if(pCore->startedTyping == false){
                pCore->startTime = SDL_GetTicks() / 1000; // start measuring time once started typing the word
                pCore->startedTyping = true;
            }
            if(pCore->inpStrLen + strlen(event->text.text) < MAXINPSTR){
                strcat(pCore->inputString, event->text.text);
                pCore->inpStrLen = strlen(pCore->inputString);
                if(pCore->pInputText) destroyText(pCore->pInputText);
                checkSpelling(pCore);
                if(pCore->isWrongLetterTyped == false){
                    pCore->pInputText = createText(pCore->pRenderer, 255, 246, 246, pCore->pTextFont, pCore->inputString, pCore->window_width/6.6+17,pCore->window_height / 1.5 + 144, false);
                }
                else{ 
                    pCore->pInputText = createText(pCore->pRenderer, 34, 35, 43, pCore->pTextFont, pCore->inputString,  pCore->window_width/6.6+17,pCore->window_height / 1.5 + 144, false);
                }
            
                updateCursorPosition(pCore);

            }
            if(event->text.text[0] == ' '){
                pCore->inputString[pCore->inpStrLen-1] = '\0';
                if(strcmp(pCore->inputString, pCore->tData.words[pCore->tData.currentWordIndex]) == 0){
                    setWordGreen(pCore);  // make word green
                    pCore->tData.currentWordIndex++;
                    if(pCore->tData.currentWordIndex == pCore->tData.nrOfWords){
                        printf("****FINISHED TEXT****\n");
                        pCore->isTextFinished = true;
                    }
                    calculateWPM(pCore); // calculate WPM and send to server
                    SDL_memset(pCore->inputString, 0, sizeof(pCore->inputString));
                    pCore->inpStrLen = 0;
                    if(pCore->pInputText) destroyText(pCore->pInputText);
                    pCore->pInputText = NULL;
                    updateCursorPosition(pCore);
                    checkSpelling(pCore);
                    return 1;
                }
                SDL_memset(pCore->inputString, 0, sizeof(pCore->inputString));
                pCore->inpStrLen = 0;
                if(pCore->pInputText) destroyText(pCore->pInputText);
                pCore->pInputText = NULL;
                updateCursorPosition(pCore);
                checkSpelling(pCore);
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if(event->key.scancode == SDL_SCANCODE_BACKSPACE){
                if(pCore->inpStrLen > 0){
                    pCore->inpStrLen--;
                    pCore->inputString[pCore->inpStrLen] = '\0';
                    if(pCore->pInputText) destroyText(pCore->pInputText);
                    if(pCore->inpStrLen > 0){
                        checkSpelling(pCore);
                        if(pCore->isWrongLetterTyped == false)
                        pCore->pInputText = createText(pCore->pRenderer, 255, 246, 246, pCore->pTextFont, pCore->inputString,  pCore->window_width/6.6+17,pCore->window_height / 1.5 + 144, false);
                        else if(pCore->isWrongLetterTyped == true){ 
                            pCore->pInputText = createText(pCore->pRenderer, 34, 35, 43, pCore->pTextFont, pCore->inputString,  pCore->window_width/6.6+17,pCore->window_height / 1.5 + 144, false);
                        }
                    } else {
                        pCore->pInputText = NULL;
                    }

                    updateCursorPosition(pCore);

                }
            }
          
    }
    return 0;
}

bool isTextFinishedCheck(GameCore *pCore){
    return pCore->isTextFinished;
}

void checkSpelling(GameCore *pCore){
    pCore->isWrongLetterTyped = false;
    for(int i = 0; i < pCore->inpStrLen; i++){
        if(pCore->inputString[i] != pCore->tData.words[pCore->tData.currentWordIndex][i]){
            printf("Wrong letter\n");
            pCore->isWrongLetterTyped = true;
            return;
        }
    }
}


char *getWPM(GameCore *pCore){
    return pCore->wpm;
}

void setWordGreen(GameCore *pCore){
    Text *completedWordText = pCore->pTextAsWords[pCore->tData.currentWordIndex];
    SDL_FRect *completedWordRect = getRect(completedWordText); 
    float savedX = completedWordRect->x;
    float savedY = completedWordRect->y;
    
    if(pCore->pTextAsWords[pCore->tData.currentWordIndex]) destroyText(pCore->pTextAsWords[pCore->tData.currentWordIndex]);
    pCore->pTextAsWords[pCore->tData.currentWordIndex] = createText(pCore->pRenderer,0,148,0,pCore->pTextFont, pCore->tData.words[pCore->tData.currentWordIndex], savedX, savedY, false);
}


void renderBlinkingCursor(GameCore *pCore){
    SDL_SetRenderDrawColor(pCore->pRenderer,220,254,230,255); 
    SDL_RenderFillRect(pCore->pRenderer, &pCore->blinkingCursor);
}


int readFromFile(GameCore *pCore, int textToLoad){
    pCore->tData.chosenText = textToLoad;
    printf("Text number %d was chosen\n", pCore->tData.chosenText);
    FILE *fp;
    fp = fopen("lib/resources/typeracertexts.txt", "r");
    if(fp == NULL){
        printf("Error opening text file %s: \n", SDL_GetError());
        return 0;
    }
    int fileTextNumber;
    char buffer[MAX_TEXT_LEN]; 
    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        if(sscanf(buffer, "%d %[^\n]", &fileTextNumber, pCore->tData.text) == 2){
            if(fileTextNumber == pCore->tData.chosenText){
                printf("Loaded text %d\n", fileTextNumber);
                fclose(fp);
                return 1;
            }
        }
    }
    printf("Text number %d was not found\n", pCore->tData.chosenText);
    fclose(fp);
    return 0;
}

void renderText(GameCore *pCore){
    for(int i = 0; i < pCore->tData.nrOfWords; i++){
        if(pCore->pTextAsWords[i]){
            drawText(pCore->pTextAsWords[i]);
        }
    }    
}

void renderInput(GameCore *pCore){
    if(pCore->pInputText) drawText(pCore->pInputText);
}

void renderHighlightRectangle(GameCore *pCore){
    SDL_SetRenderDrawColor(pCore->pRenderer, 66,66,66,255);
    SDL_RenderRect(pCore->pRenderer, &pCore->highlightWindow);

    SDL_SetRenderDrawColor(pCore->pRenderer, 40, 40, 40, 255);
    SDL_RenderFillRect(pCore->pRenderer, &pCore->lowerBoxRect);
    SDL_SetRenderDrawColor(pCore->pRenderer, 255,255,255,255);
    SDL_RenderRect(pCore->pRenderer, &pCore->highlightInputBox);
    SDL_SetRenderDrawColor(pCore->pRenderer, 66,66,66,255);
    SDL_RenderRect(pCore->pRenderer, &pCore->highlightLowerBox);
    



}

void renderRectangle(GameCore *pCore){
    if(pCore->isWrongLetterTyped == true){
        SDL_SetRenderDrawColor(pCore->pRenderer, 200, 80, 80 ,180); // soft red 
        SDL_RenderFillRect(pCore->pRenderer, &pCore->inputBox);
    }
    else{
        SDL_SetRenderDrawColor(pCore->pRenderer, 33, 33, 33, 255);
        SDL_RenderFillRect(pCore->pRenderer, &pCore->inputBox);
    }
   
}

void createNamesAndWPM(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
       
        int textWidth;
        TTF_GetStringSize(pCore->pNamesFont, pCore->gData_local.players[i].playerName, 0, &textWidth, NULL);
        
        // Calculate position so text ends near car start X
        int carStartX = 380;  
        int padding = 15;    
        int nameX = carStartX - padding - textWidth; 
        
        if(nameX < 50) nameX = 50;
        
        pCore->pNames[i] = createText(pCore->pRenderer, 247, 255, 255, pCore->pNamesFont, pCore->gData_local.players[i].playerName, nameX, 120 + i*85, false);
        
        strcpy(pCore->gData_local.players[i].WPM, "0");
        pCore->pWPM[i] = createText(pCore->pRenderer, 255, 255, 255, pCore->pTextFont, pCore->gData_local.players[i].WPM, pCore->window_width - 370, 140 + i*85, true);
        pCore->pWPMText[i] = createText(pCore->pRenderer, 255, 255, 255, pCore->pWpmTextFont,"wpm", pCore->window_width - 300, 140 + i*85, true);
    }
}

void renderCars(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        if(pCore->pCars[i]) SDL_RenderTexture(pCore->pRenderer, pCore->pCars[i], NULL, &pCore->cars[i]);
    }
}

void setGameCoreChanged(GameCore *pCore, bool changed){
    pCore->isGameChanged = changed;
}

void updateGameCore(GameCore *pCore){
    if(pCore->isGameChanged){
        for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
            if(pCore->pWPM[i]) destroyText(pCore->pWPM[i]);
            pCore->pWPM[i] = NULL;
        }
        for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
            pCore->pWPM[i] = createText(pCore->pRenderer, 255 , 255 , 255, pCore->pTextFont, pCore->gData_local.players[i].WPM,pCore->window_width - 370, 140 + i*85, true );
            if(pCore->gData_local.players[i].placement[0] != 0){
                if(strcmp(pCore->gData_local.players[i].placement, "1st") == 0){
                    pCore->pPlacements[i] = createText(pCore->pRenderer, 234, 200 ,0, pCore->pPlacementFont, pCore->gData_local.players[i].placement, pCore->window_width - 550, 140 + i * 85, true );
                }
                if(strcmp(pCore->gData_local.players[i].placement, "2nd") == 0){
                    pCore->pPlacements[i] = createText(pCore->pRenderer, 192, 192, 192, pCore->pPlacementFont, pCore->gData_local.players[i].placement, pCore->window_width - 550, 140 + i * 85, true );
                }
                if(strcmp(pCore->gData_local.players[i].placement, "3rd") == 0){
                    pCore->pPlacements[i] = createText(pCore->pRenderer, 205, 127, 50, pCore->pPlacementFont, pCore->gData_local.players[i].placement,pCore->window_width - 550, 140 + i * 85, true );
                }
                if(strcmp(pCore->gData_local.players[i].placement, "4th") == 0){
                    pCore->pPlacements[i] = createText(pCore->pRenderer, 128, 128, 128, pCore->pPlacementFont, pCore->gData_local.players[i].placement, pCore->window_width - 550, 140 + i * 85, true );
                }
               
            }
        

        }
        updateCars(pCore);

        pCore->isGameChanged = false;
    }
}

void renderPlacements(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        if(pCore->pPlacements[i]) drawText(pCore->pPlacements[i]);
    }
}

void renderWPM(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers;i++){
        if(pCore->pWPM[i]) drawText(pCore->pWPM[i]);
        if(pCore->pWPMText[i]) drawText(pCore->pWPMText[i]);
    }
}



void renderCore(GameCore *pCore, Audio *pAudio){
    renderPlacements(pCore);
    renderWPM(pCore);
    renderHighlightRectangle(pCore);
    renderText(pCore);
    renderRectangle(pCore);
    renderInput(pCore);
    renderNames(pCore);
    renderCars(pCore);
    renderBlinkingCursor(pCore);
        drawLines(pCore);

   if(!pCore->countdownFinished){
        renderCountdown(pCore, pAudio);
    }
    

}

void renderCountdown(GameCore *pCore, Audio *pAudio){
    int currentTime = SDL_GetTicks()/1000;
    int elapsed = currentTime - pCore->startTimeForCounter;
    
    int textIndex = -1; 
    if(elapsed < 2) {
        textIndex = 0;      // 5
        if(pCore->playSound == true) playCountdown(pAudio); pCore->playSound = false;
    }
    else if(elapsed < 3) {
        textIndex = 1;      // 4
        if(pCore->playSound == false) playCountdown(pAudio); pCore->playSound = true;

    }
    else if(elapsed < 4) {

        textIndex = 2;      // 3
        if(pCore->playSound == true) playCountdown(pAudio); pCore->playSound = false;
    }
    else if(elapsed < 5) {

        textIndex = 3;      // 2
        if(pCore->playSound == false) playCountdown(pAudio); pCore->playSound = true;
    }
    else if(elapsed < 6) {

        textIndex = 4;      // 1
        if(pCore->playSound == true) playCountdown(pAudio); pCore->playSound = false;

    }
    else if(elapsed < 7) {
        textIndex = 5;      // GO!
        if(pCore->playSound == false) playCountdownFinish(pAudio); pCore->playSound = true;
    }
    else {
        SDL_StartTextInput(pCore->pWindow);
        pCore->countdownFinished = true;
        return;
    }
    
    if(textIndex >= 0 && pCore->pCountdownText[textIndex]) {
        drawText(pCore->pCountdownText[textIndex]);
    }
}

void renderNames(GameCore *pCore){
    for(int i = 0; i < pCore->gData_local.nrOfPlayers; i++){
        drawText(pCore->pNames[i]);
    }
}


GameCoreData *getGData_local(GameCore *pCore){
    return &pCore->gData_local;
}

void destroyGameCore(GameCore *pCore){
      for(int i = 0; i < pCore->gData_local.nrOfPlayers;i++){
        if(pCore->pWPMText[i]) destroyText(pCore->pWPMText[i]);
        if(pCore->pPlacements[i]) destroyText(pCore->pPlacements[i]);
        if(pCore->pWPM[i]) destroyText(pCore->pWPM[i]);
    }
    if(pCore->pInputText) destroyText(pCore->pInputText);
    for(int i = 0; i < MAXTEXTWORD; i++){
        if(pCore->pTextAsWords[i]) destroyText(pCore->pTextAsWords[i]);
    }
    if(pCore->pNamesFont) TTF_CloseFont(pCore->pNamesFont);
    if(pCore->pTextFont) TTF_CloseFont(pCore->pTextFont);
    for (int i = 0; i < MAXCLIENTS; i++){
        if (pCore->pCars[i])
            SDL_DestroyTexture(pCore->pCars[i]);
        if(pCore->pNames[i]){
            destroyText(pCore->pNames[i]);
        }
    }
    free(pCore);
}