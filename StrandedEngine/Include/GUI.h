/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#ifndef _UGP_D3D_GUI_H_
#define _UGP_D3D_GUI_H_

#include"defines.h"


struct stGUIControl
{
   // Control type, id, and color.
   int m_type, m_id;
   unsigned long m_color;

   // If text then this is the font id, else static buffer id.
   int m_listID;

   // Start pos of object, width and height of button.
   int m_xPos, m_yPos, m_width, m_height;

   // Text of static text.
   char *m_text;

   // Button's up, down, and over textures.
   // m_upTex also used for backdrop image.
   int m_upTex, m_downTex, m_overTex;
};


class CGUISystem
{
   public:
      CGUISystem() : m_controls(0), m_totalControls(0),
                     m_backDropID(-1) {}
      ~CGUISystem() { Shutdown(); }

      int IncreaseControls();

      bool AddBackdrop(int texID, int staticID);
      bool AddStaticText(int id, char *text, int x, int y,
                         unsigned long color, int fontID);
      bool AddButton(int id, int x, int y, int width,
                     int height, int upID, int overID,
                     int downID, unsigned int staticID);
      void Shutdown();

      stGUIControl *GetGUIControl(int id)
      {
         if(id < 0 || id >= m_totalControls) return NULL;
         return &m_controls[id];
      }

      int GetTotalControls()
      {
         return m_totalControls;
      }

      stGUIControl *GetBackDrop()
      {
         if(m_backDropID >= 0 && m_totalControls)
            return &m_controls[m_backDropID];

         return NULL;
      }

   private:
      stGUIControl *m_controls;
      int m_totalControls;
      int m_backDropID;
};

#endif