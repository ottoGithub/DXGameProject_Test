/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#include"main.h"


// Globals...
HWND g_hwnd;
CRenderInterface *g_Render = NULL;

// GUI ids.
int g_mainGui = -1;
int g_startGui = -1;
int g_creditsGui = -1;
int g_currentGUI = GUI_MAIN_SCREEN;

// Font id.
int g_arialID = -1;

// Temp Mouse state information.
bool LMBDown = false;
int mouseX = 0, mouseY = 0;

void ShowLog();


LRESULT WINAPI MsgProc(HWND hd, UINT msg, WPARAM wp, LPARAM lp)
{
   switch(msg)
      {
         case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            break;

         case WM_KEYUP:
            if(wp == VK_ESCAPE) PostQuitMessage(0);
            break;

		 case WM_LBUTTONDOWN:
			LMBDown = true;
			//MainMenuRender();
            break;

         case WM_LBUTTONUP:
            LMBDown = false;
            break;

         case WM_MOUSEMOVE:
            mouseY = HIWORD (lp);
            mouseX = LOWORD (lp);
            break;
      }

   return DefWindowProc(hd, msg, wp, lp);
}


int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR cmd, int show)
{
   // Register the window class
   WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc,
                     0L, 0L, GetModuleHandle(NULL), NULL, NULL,
                     NULL, NULL, WINDOW_CLASS, NULL };
   RegisterClassEx(&wc);

   // Create the application's window
   if(FULLSCREEN)
      {
		 g_hwnd = CreateWindowEx(NULL, WINDOW_CLASS, WINDOW_NAME,
                        WS_POPUP | WS_SYSMENU | WS_VISIBLE, 0, 0,
                        WIN_WIDTH, WIN_HEIGHT,
                        NULL, NULL, h, NULL);
      }
   else
      {
		 const int sys_min_height = GetSystemMetrics(SM_CYMIN);  
		 const int sys_caption_height = GetSystemMetrics(SM_CYSMCAPTION);  
		 g_hwnd = CreateWindowEx(NULL, WINDOW_CLASS, WINDOW_NAME,
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0,
                              0, WIN_WIDTH + (sys_min_height - sys_caption_height),
							  WIN_HEIGHT + sys_min_height,
                              NULL, NULL, h, NULL);
      }

   if(g_hwnd)
      {
         // Show the window
         ShowWindow(g_hwnd, SW_SHOWDEFAULT);
         UpdateWindow(g_hwnd);
      }

   // Initialize the Stranded Engine.
   if(InitializeEngine())
      {
         // Initialize Stranded game.
         if(GameInitialize())
            {
               // Enter the message loop
               MSG msg;
               ZeroMemory(&msg, sizeof(msg));

               SetCursorPos(0, 0);

               while(msg.message != WM_QUIT)
                  {
                     if(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                        {
                           TranslateMessage(&msg);
                           DispatchMessage(&msg);
                        }
                     else
                        GameLoop();
                  }
            }
      }

   // Release any and all resources.
   GameShutdown();
   ShutdownEngine();

   UnregisterClass(WINDOW_CLASS, wc.hInstance);
   return 0;
}


bool InitializeEngine()
{
   if(!CreateD3DRenderer(&g_Render)) return false;

   if(!g_Render->Initialize(WIN_WIDTH, WIN_HEIGHT,
      g_hwnd, FULLSCREEN)) return false;

   g_Render->SetClearCol(0, 0, 0);

   if(!g_Render->CreateText("Times New Roman", 0, false, 18, g_arialID))
      return false;

   return true;
}


void ShutdownEngine()
{
   if(g_Render)
      {
         g_Render->Shutdown();
         delete g_Render;
         g_Render = NULL;
      }
}


bool InitializeMainMenu()
{
   // Create gui screens.
   if(!g_Render->CreateGUI(g_mainGui)) return false;
   if(!g_Render->CreateGUI(g_startGui)) return false;
   if(!g_Render->CreateGUI(g_creditsGui)) return false;

   // Load backdrops.
   if(!g_Render->AddGUIBackdrop(g_mainGui, "menu/mainMenu.jpg"))
      return false;
   if(!g_Render->AddGUIBackdrop(g_startGui, "menu/startMenu.jpg"))
      return false;
   if(!g_Render->AddGUIBackdrop(g_creditsGui,
      "menu/creditsMenu.jpg")) return false;

   // Set main screen elements.
   if(!g_Render->AddGUIStaticText(g_mainGui, STATIC_TEXT_ID,
      "Version: 1.0", PERCENT_OF(WIN_WIDTH, 0.85),
      PERCENT_OF(WIN_WIDTH, 0.05),
      UGPCOLOR_ARGB(255,255,255,255), g_arialID)) return false;

   if(!g_Render->AddGUIButton(g_mainGui, BUTTON_START_ID,
      PERCENT_OF(WIN_WIDTH, 0.05), PERCENT_OF(WIN_HEIGHT, 0.40),
      "menu/startUp.png", "menu/StartOver.png",
      "menu/startDown.png")) return false;

   if(!g_Render->AddGUIButton(g_mainGui, BUTTON_CREDITS_ID,
      PERCENT_OF(WIN_WIDTH, 0.05), PERCENT_OF(WIN_HEIGHT, 0.50),
      "menu/creditsUp.png", "menu/creditsOver.png",
      "menu/creditsDown.png")) return false;

   if(!g_Render->AddGUIButton(g_mainGui, BUTTON_QUIT_ID,
      PERCENT_OF(WIN_WIDTH, 0.05), PERCENT_OF(WIN_HEIGHT, 0.60),
      "menu/quitUp.png", "menu/quitOver.png",
      "menu/quitDown.png")) return false;


   // Set start screen elements.
   if(!g_Render->AddGUIButton(g_startGui, BUTTON_LEVEL_1_ID,
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.15),
      "menu/level1Up.png", "menu/level1Over.png",
      "menu/level1Down.png")) return false;

   if(!g_Render->AddGUIButton(g_startGui, BUTTON_BACK_ID,
	   PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.80),
	   "menu/backUp.png", "menu/backOver.png",
	   "menu/backDown.png")) return false;
   //if(!g_Render->AddGUIButton(g_startGui, BUTTON_BACK_ID,
   // PERCENT_OF(WIN_WIDTH, 0.0), PERCENT_OF(WIN_HEIGHT, 0.0),
   // "menu/backUp.png", "menu/backOver.png",
   // "menu/backDown.png")) return false;

   // Set credits screen elements.
   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "Game Design -",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.15),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Allen Sherrod",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.20),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "Programming -",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.25),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Allen Sherrod",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.30),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "Sound -",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.35),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Allen Sherrod",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.40),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "Level Design -",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.45),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Allen Sherrod",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.50),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "Speical Thanks -",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.55),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Jenifer Niles",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.60),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Bryan Davidson",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.65),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Charles River Media",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.70),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID,
      "              Readers of this book",
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.75),
      UGPCOLOR_ARGB(255,255,255,255),
      g_arialID)) return false;

   if(!g_Render->AddGUIButton(g_creditsGui, BUTTON_BACK_ID,
      PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.80),
      "menu/backUp.png", "menu/backOver.png",
      "menu/backDown.png")) return false;

   return true;
}


void MainMenuCallback(int id, int state)
{
   switch(id)
      {
         case BUTTON_START_ID:
            if(state == UGP_BUTTON_DOWN)
               g_currentGUI = GUI_START_SCREEN;
            break;

         case BUTTON_CREDITS_ID:
            if(state == UGP_BUTTON_DOWN)
               g_currentGUI = GUI_CREDITS_SCREEN;
            break;

         case BUTTON_BACK_ID:
            if(state == UGP_BUTTON_DOWN)
               g_currentGUI = GUI_MAIN_SCREEN;
            break;

         case BUTTON_QUIT_ID:
            if(state == UGP_BUTTON_DOWN)
               PostQuitMessage(0);
            break;

         case BUTTON_LEVEL_1_ID:
            // Start first level.
            break;
      }
}


void MainMenuRender()
{
   if(!g_Render) return;

   g_Render->StartRender(1, 1, 0);

      if(g_currentGUI == GUI_MAIN_SCREEN)
         g_Render->ProcessGUI(g_mainGui, LMBDown, mouseX,
                              mouseY, MainMenuCallback);
      else if(g_currentGUI == GUI_START_SCREEN)
         g_Render->ProcessGUI(g_startGui, LMBDown, mouseX,
                              mouseY, MainMenuCallback);
      else
         g_Render->ProcessGUI(g_creditsGui, LMBDown, mouseX,
                              mouseY, MainMenuCallback);


	ShowLog();

   g_Render->EndRendering();
}


bool GameInitialize()
{
   if(!InitializeMainMenu()) return false;

   return true;
}

void ShowLog()
{
	if(!g_Render)
	{
		return;
	}
	char szLog[128] = {0};
	sprintf(szLog, "%d,%d",mouseX,mouseY);
	g_Render->DisplayText(0,400,0,UGPCOLOR_ARGB(255,0,128,255),szLog);
}


void GameLoop()
{
   MainMenuRender();
}


void GameShutdown()
{

}