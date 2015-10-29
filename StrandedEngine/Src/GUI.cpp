/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#include"GUI.h"


int CGUISystem::IncreaseControls()
{
   // This function increases the m_control array.

   if(!m_controls)
      {
         m_controls = new stGUIControl[1];
         if(!m_controls) return UGP_FAIL;
         memset(&m_controls[0], 0, sizeof(stGUIControl));
      }
   else
      {
         stGUIControl *temp;
         temp = new stGUIControl[m_totalControls + 1];
         if(!temp) return UGP_FAIL;
         memset(temp, 0, sizeof(stGUIControl) *
                (m_totalControls + 1));

         memcpy(temp, m_controls,
               sizeof(stGUIControl) * m_totalControls);

         delete[] m_controls;
         m_controls = temp;
      }

   return UGP_OK;
}


bool CGUISystem::AddBackdrop(int texID, int sID)
{
   if(texID < 0 || sID < 0) return false;

   if(m_backDropID < 0)
      {
         // Create a blank control.
         if(!IncreaseControls()) return false;

         // Fill in necessary info.
         m_controls[m_totalControls].m_type = UGP_GUI_BACKDROP;
         m_controls[m_totalControls].m_upTex = texID;
         m_controls[m_totalControls].m_listID = sID;

         // Keep track which index is backdrop.
         // Allows us to render backdrop first without
         // having to have added it first.
         m_backDropID = m_totalControls;

         // Increment total.
         m_totalControls++;
      }
   else
      {
         // Else just override the tex id and static buffer id.
         m_controls[m_backDropID].m_upTex = texID;
         m_controls[m_backDropID].m_listID = sID;
      }

   return true;
}


bool CGUISystem::AddStaticText(int id, char *text, int x, int y,
                               unsigned long color, int fontID)
{
   if(!text || fontID < 0) return false;

   // Create a blank control.
   if(!IncreaseControls()) return false;

   // Fill it with all the info we need for static text.
   m_controls[m_totalControls].m_type = UGP_GUI_STATICTEXT;
   m_controls[m_totalControls].m_id = id;
   m_controls[m_totalControls].m_color = color;
   m_controls[m_totalControls].m_xPos = x;
   m_controls[m_totalControls].m_yPos = y;
   m_controls[m_totalControls].m_listID = fontID;

   // Copy text data.
   int len = strlen(text);
   m_controls[m_totalControls].m_text = new char[len + 1];
   if(!m_controls[m_totalControls].m_text) return false;
   memcpy(m_controls[m_totalControls].m_text, text, len); 
   m_controls[m_totalControls].m_text[len] = '\0';

   // Increment total.
   m_totalControls++;

   return true;
}


bool CGUISystem::AddButton(int id, int x, int y, int width,
   int height, int upID, int overID,
   int downID, unsigned int staticID)
{
   // Create a blank control.
   if(!IncreaseControls()) return false;

   // Set all the data needed to render/process a button.
   m_controls[m_totalControls].m_type = UGP_GUI_BUTTON;
   m_controls[m_totalControls].m_id = id;
   m_controls[m_totalControls].m_xPos = x;
   m_controls[m_totalControls].m_yPos = y;
   m_controls[m_totalControls].m_width = width;
   m_controls[m_totalControls].m_height = height;
   m_controls[m_totalControls].m_upTex = upID;
   m_controls[m_totalControls].m_overTex = overID;
   m_controls[m_totalControls].m_downTex = downID;
   m_controls[m_totalControls].m_listID = staticID;

   // Increment total.
   m_totalControls++;

   return true;
}


void CGUISystem::Shutdown()
{
   // Release all resources.
   for(int s = 0; s < m_totalControls; s++)
      {
         if(m_controls[s].m_text)
            {
               delete[] m_controls[s].m_text;
               m_controls[s].m_text = NULL;
            }
      }

   m_totalControls = 0;
   if(m_controls) delete[] m_controls;
   m_controls = NULL;
}