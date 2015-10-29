/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#ifndef _UGP_MAIN_H_
#define _UGP_MAIN_H_


#include "engine.h"
//#pragma comment(lib, "lib/StrandedEngine.lib")

#define WINDOW_CLASS    "StrandedGame"
#define WINDOW_NAME     "Stranded"
#define WIN_WIDTH       800
#define WIN_HEIGHT      600
#define FULLSCREEN      0

// Function Prototypes...
bool InitializeEngine();
void ShutdownEngine();

// Menu functions.
bool InitializeMainMenu();
void MainMenuCallback(int id, int state);
void MainMenuRender();

// Main game functions.
bool GameInitialize();
void GameLoop();
void GameShutdown();

// Main menu defines.
#define GUI_MAIN_SCREEN       1
#define GUI_START_SCREEN      2
#define GUI_CREDITS_SCREEN    3

// ids for our GUI controls.
#define STATIC_TEXT_ID     1
#define BUTTON_START_ID    2
#define BUTTON_CREDITS_ID  3
#define BUTTON_QUIT_ID     4
#define BUTTON_BACK_ID     5
#define BUTTON_LEVEL_1_ID  6

#endif