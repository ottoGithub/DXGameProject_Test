/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#include<stdio.h>
#include"D3DRenderer.h"
#include"structs.h"


inline unsigned long FtoDW(float val)
{
   return *((unsigned long*)&val);
}


// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_GUI (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)


bool CreateD3DRenderer(CRenderInterface **pObj)
{
   if(!*pObj) *pObj = new CD3DRenderer;
   else return false;
   
   return true;
}


unsigned long CreateD3DFVF(int flags)
{
   unsigned long fvf = 0;

   if(flags == GUI_FVF) fvf = D3DFVF_GUI;

   return fvf;
}


CD3DRenderer::CD3DRenderer()
{
   m_Direct3D = NULL;
   m_Device = NULL;
   m_renderingScene = false;
   m_numStaticBuffers = 0;
   m_activeStaticBuffer = UGP_INVALID;
   m_staticBufferList = NULL;
   m_textureList = NULL;
   m_numTextures = 0;
   m_totalFonts = 0;
   m_guiList = NULL;
   m_totalGUIs = 0;
   m_fonts = NULL;
}


CD3DRenderer::~CD3DRenderer()
{
   Shutdown();
}


bool CD3DRenderer::Initialize(int w, int h, WinHWND mainWin,
                              bool fullScreen)
{
   Shutdown();

   m_mainHandle = mainWin;
   if(!m_mainHandle) return false;

   m_fullscreen = fullScreen;

   D3DDISPLAYMODE mode;
   D3DCAPS9 caps;
   D3DPRESENT_PARAMETERS Params;

   ZeroMemory(&Params, sizeof(Params));

   m_Direct3D = Direct3DCreate9(D3D_SDK_VERSION);
   if(!m_Direct3D) return false;

   if(FAILED(m_Direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
             &mode))) return false;

	if(FAILED(m_Direct3D->GetDeviceCaps(D3DADAPTER_DEFAULT,
	          D3DDEVTYPE_HAL, &caps))) return false;

	DWORD processing = 0;
	if(caps.VertexProcessingCaps != 0)
		processing = D3DCREATE_HARDWARE_VERTEXPROCESSING |
		             D3DCREATE_PUREDEVICE;
	else
		processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

   if(m_fullscreen)
      {
         Params.FullScreen_RefreshRateInHz = mode.RefreshRate;
         Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
      }
   else
      Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

   Params.Windowed = !m_fullscreen;
   Params.BackBufferWidth = w;
   Params.BackBufferHeight = h;
   Params.hDeviceWindow = m_mainHandle;
   Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
   Params.BackBufferFormat = mode.Format;
   Params.BackBufferCount = 1;
   Params.EnableAutoDepthStencil = TRUE;
   Params.AutoDepthStencilFormat = D3DFMT_D16;

   m_screenWidth = w;
   m_screenHeight = h;

   if(FAILED(m_Direct3D->CreateDevice(D3DADAPTER_DEFAULT,
             D3DDEVTYPE_HAL, m_mainHandle, processing,
             &Params, &m_Device))) return false;

   if(m_Device == NULL) return false;

   OneTimeInit();
   return true;
}


void CD3DRenderer::OneTimeInit()
{
   if(!m_Device) return;

   m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

   SetTextureFilter(0, MAG_FILTER, ANISOTROPIC_TYPE);
   SetTextureFilter(0, MIN_FILTER, ANISOTROPIC_TYPE);
   SetTextureFilter(0, MIP_FILTER, ANISOTROPIC_TYPE);

   CalculateProjMatrix(D3DX_PI / 4, 0.1f, 1000);
}


void CD3DRenderer::Shutdown()
{
   for(int s = 0; s < m_numStaticBuffers; s++)
      {
         if(m_staticBufferList[s].vbPtr)
            {
               m_staticBufferList[s].vbPtr->Release();
               m_staticBufferList[s].vbPtr = NULL;
            }
         
         if(m_staticBufferList[s].ibPtr)
            {
               m_staticBufferList[s].ibPtr->Release();
               m_staticBufferList[s].ibPtr = NULL;
            }
      }

   m_numStaticBuffers = 0;
   if(m_staticBufferList) delete[] m_staticBufferList;
   m_staticBufferList = NULL;


   for(int s = 0; s < m_numTextures; s++)
      {
         if(m_textureList[s].fileName)
            {
               delete[] m_textureList[s].fileName;
               m_textureList[s].fileName = NULL;
            }

         if(m_textureList[s].image)
            {
               m_textureList[s].image->Release();
               m_textureList[s].image = NULL;
            }
      }

   m_numTextures = 0;
   if(m_textureList) delete[] m_textureList;
   m_textureList = NULL;


   for(int s = 0; s < m_totalFonts; s++)
      {
         if(m_fonts[s])
            {
               m_fonts[s]->Release();
               m_fonts[s] = NULL;
            }
      }

   m_totalFonts = 0;
   if(m_fonts) delete[] m_fonts;
   m_fonts = NULL;

   for(int s = 0; s < m_totalGUIs; s++)
      {
         m_guiList[s].Shutdown();
      }

   m_totalGUIs = 0;
   if(m_guiList) delete[] m_guiList;
   m_guiList = NULL;


   if(m_Device) m_Device->Release();
   if(m_Direct3D) m_Direct3D->Release();
   
   m_Device = NULL;
   m_Direct3D = NULL;
}


void CD3DRenderer::SetClearCol(float r, float g, float b)
{
   m_Color = D3DCOLOR_COLORVALUE(r, g, b, 1.0f);
}


void CD3DRenderer::StartRender(bool bColor, bool bDepth,
                               bool bStencil)
{
   if(!m_Device) return;

   unsigned int buffers = 0;
   if(bColor) buffers |= D3DCLEAR_TARGET;
   if(bDepth) buffers |= D3DCLEAR_ZBUFFER;
   if(bStencil) buffers |= D3DCLEAR_STENCIL;

   if(FAILED(m_Device->Clear(0, NULL, buffers, m_Color, 1, 0)))
      return;
   if(FAILED(m_Device->BeginScene())) return;
 
   m_renderingScene = true;
}


void CD3DRenderer::ClearBuffers(bool bColor, bool bDepth,
                                bool bStencil)
{
   if(!m_Device) return;

   unsigned int buffers = 0;
   if(bColor) buffers |= D3DCLEAR_TARGET;
   if(bDepth) buffers |= D3DCLEAR_ZBUFFER;
   if(bStencil) buffers |= D3DCLEAR_STENCIL;

   if(m_renderingScene) m_Device->EndScene();
   if(FAILED(m_Device->Clear(0, NULL, buffers, m_Color, 1, 0)))
      return;

   if(m_renderingScene)
      if(FAILED(m_Device->BeginScene())) return;
}


void CD3DRenderer::EndRendering()
{
   if(!m_Device) return;

   m_Device->EndScene();
   m_Device->Present(NULL, NULL, NULL, NULL);

   m_renderingScene = false;
}


void CD3DRenderer::SetMaterial(stMaterial *mat)
{
   if(!mat || !m_Device) return;

   D3DMATERIAL9 m = { mat->diffuseR, mat->diffuseG,
                     mat->diffuseB, mat->diffuseA,
                     mat->ambientR, mat->ambientG,
                     mat->ambientB, mat->ambientA,
                     mat->specularR, mat->specularG,
                     mat->specularB, mat->specularA,
                     mat->emissiveR, mat->emissiveG,
                     mat->emissiveB, mat->emissiveA,
                     mat->power
   };

   m_Device->SetMaterial(&m);
}


void CD3DRenderer::SetLight(stLight *light, int index)
{
   if(!light || !m_Device || index < 0) return;

   D3DLIGHT9 l;

   l.Ambient.a = light->ambientA;
   l.Ambient.r = light->ambientR;
   l.Ambient.g = light->ambientG;
   l.Ambient.b = light->ambientB;
   l.Attenuation0 = light->attenuation0;
   l.Attenuation1 = light->attenuation1;
   l.Attenuation2 = light->attenuation2;
   l.Diffuse.a = light->diffuseA;
   l.Diffuse.r = light->diffuseR;
   l.Diffuse.g = light->diffuseG;
   l.Diffuse.b = light->diffuseB;
   l.Direction.x = light->dirX;
   l.Direction.y = light->dirY;
   l.Direction.z = light->dirZ;
   l.Falloff = light->falloff;
   l.Phi = light->phi;
   l.Position.x = light->posX;
   l.Position.y = light->posY;
   l.Position.z = light->posZ;
   l.Range = light->range;
   l.Specular.a = light->specularA;
   l.Specular.r = light->specularR;
   l.Specular.g = light->specularG;
   l.Specular.b = light->specularB;
   l.Theta = light->theta;


   if(light->type == LIGHT_POINT) l.Type = D3DLIGHT_POINT;
   else if (light->type == LIGHT_SPOT) l.Type = D3DLIGHT_SPOT;
   else l.Type = D3DLIGHT_DIRECTIONAL;

   m_Device->SetLight(index, &l);
   m_Device->LightEnable(index, TRUE);
}


void CD3DRenderer::DisableLight(int index)
{
   if(!m_Device) return;

   m_Device->LightEnable(index, FALSE);
}


void CD3DRenderer::SetDepthTesting(RenderState state)
{
   if(!m_Device) return;

   if(state == DEPTH_NONE)
      {
         m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
         m_Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
      }
   else if(state == DEPTH_READONLY)
      {
         m_Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
         m_Device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
      }
   else if(state == DEPTH_READWRITE)
      {
         m_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
         m_Device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
      }
}


void CD3DRenderer::SetTransparency(RenderState state, TransState src,
                                   TransState dst)
{
   if(!m_Device) return;

   if(state == TRANSPARENCY_NONE)
      {
         m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
         return;
      }

   if(state == TRANSPARENCY_ENABLE)
      {
         m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

         switch(src)
            {
               case TRANS_ZERO:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_ZERO);
                  break;
               
               case TRANS_ONE:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_ONE);
                  break;
               
               case TRANS_SRCCOLOR:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_SRCCOLOR);
                  break;
               
               case TRANS_INVSRCCOLOR:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_INVSRCCOLOR);
                  break;
               
               case TRANS_SRCALPHA:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_SRCALPHA);
                  break;
               
               case TRANS_INVSRCALPHA:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_INVSRCALPHA);
                  break;
                  
               case TRANS_DSTALPHA:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_DESTALPHA);
                  break;
               
               case TRANS_INVDSTALPHA:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_INVDESTALPHA);
                  break;
               
               case TRANS_DSTCOLOR:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_DESTCOLOR);
                  break;
               
               case TRANS_INVDSTCOLOR:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_INVDESTCOLOR);
                  break;
               
               case TRANS_SRCALPHASAT:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_SRCALPHASAT);
                  break;
               
               case TRANS_BOTHSRCALPHA:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_BOTHSRCALPHA);
                  break;
               
               case TRANS_INVBOTHSRCALPHA:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_BOTHINVSRCALPHA);
                  break;
               
               case TRANS_BLENDFACTOR:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_BLENDFACTOR);
                  break;                  
               
               case TRANS_INVBLENDFACTOR:
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_INVBLENDFACTOR);
                  break;
               
               default:
                  m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE,
                                           false);
                  return;
                  break;
            }
         
         switch(dst)
            {
               case TRANS_ZERO:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_ZERO);
                  break;
               
               case TRANS_ONE:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_ONE);
                  break;
               
               case TRANS_SRCCOLOR:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_SRCCOLOR);
                  break;
               
               case TRANS_INVSRCCOLOR:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_INVSRCCOLOR);
                  break;
               
               case TRANS_SRCALPHA:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_SRCALPHA);
                  break;
               
               case TRANS_INVSRCALPHA:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_INVSRCALPHA);
                  break;
                  
               case TRANS_DSTALPHA:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_DESTALPHA);
                  break;
               
               case TRANS_INVDSTALPHA:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_INVDESTALPHA);
                  break;
               
               case TRANS_DSTCOLOR:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_DESTCOLOR);
                  break;
               
               case TRANS_INVDSTCOLOR:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_INVDESTCOLOR);
                  break;
               
               case TRANS_SRCALPHASAT:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_SRCALPHASAT);
                  break;
               
               case TRANS_BOTHSRCALPHA:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_BOTHSRCALPHA);
                  break;
               
               case TRANS_INVBOTHSRCALPHA:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_BOTHINVSRCALPHA);
                  break;
               
               case TRANS_BLENDFACTOR:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_BLENDFACTOR);
                  break;                  
               
               case TRANS_INVBLENDFACTOR:
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_INVBLENDFACTOR);
                  break;
               
               default:
                  m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE,
                                           false);
                  break;
            }
      }
}


int CD3DRenderer::AddTexture2D(char *file, int *texId)
{
   if(!file || !m_Device) return UGP_FAIL;

   int len = strlen(file);
   if(!len) return UGP_FAIL;

   int index = m_numTextures;

   if(!m_textureList)
      {
         m_textureList = new stD3DTexture[1];
         if(!m_textureList) return UGP_FAIL;
      }
   else
      {
         stD3DTexture *temp;
         temp = new stD3DTexture[m_numTextures + 1];

         memcpy(temp, m_textureList,
                sizeof(stD3DTexture) * m_numTextures);

         delete[] m_textureList;
         m_textureList = temp;
      }

   m_textureList[index].fileName = new char[len];
   memcpy(m_textureList[index].fileName, file, len);

   D3DCOLOR colorkey = 0xff000000;
   D3DXIMAGE_INFO info;

   if(D3DXCreateTextureFromFileEx(m_Device, file, 0, 0, 0, 0,
      D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT,
      D3DX_DEFAULT,  colorkey, &info, NULL,
      &m_textureList[index].image) != D3D_OK) return false;

   m_textureList[index].image->SetAutoGenFilterType(D3DTEXF_LINEAR);
   m_textureList[index].image->GenerateMipSubLevels();

   m_textureList[index].width = info.Width;
   m_textureList[index].height = info.Height;

   *texId = m_numTextures;
   m_numTextures++;

   return UGP_OK;
}


void CD3DRenderer::SetTextureFilter(int index, int filter, int val)
{
   if(!m_Device || index < 0) return;

   D3DSAMPLERSTATETYPE fil = D3DSAMP_MINFILTER;
   int v = D3DTEXF_POINT;

   if(filter == MIN_FILTER) fil = D3DSAMP_MINFILTER;
   if(filter == MAG_FILTER) fil = D3DSAMP_MAGFILTER;
   if(filter == MIP_FILTER) fil = D3DSAMP_MIPFILTER;

   if(val == POINT_TYPE) v = D3DTEXF_POINT;
   if(val == LINEAR_TYPE) v = D3DTEXF_LINEAR;
   if(val == ANISOTROPIC_TYPE) v = D3DTEXF_ANISOTROPIC;

   m_Device->SetSamplerState(index, fil, v);
}


void CD3DRenderer::SetMultiTexture()
{
   if(!m_Device) return;

   m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
   m_Device->SetTextureStageState(0, D3DTSS_COLOROP,
                                  D3DTOP_MODULATE);
   m_Device->SetTextureStageState(0, D3DTSS_COLORARG1,
                                  D3DTA_TEXTURE);
   m_Device->SetTextureStageState(0, D3DTSS_COLORARG2,
                                  D3DTA_DIFFUSE);

   m_Device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
   m_Device->SetTextureStageState(1, D3DTSS_COLOROP,
                                  D3DTOP_MODULATE);
   m_Device->SetTextureStageState(1, D3DTSS_COLORARG1,
                                  D3DTA_TEXTURE);
   m_Device->SetTextureStageState(1, D3DTSS_COLORARG2,
                                  D3DTA_CURRENT);
}


void CD3DRenderer::ApplyTexture(int index, int texId)
{
   if(!m_Device) return;

   if(index < 0 || texId < 0)
      m_Device->SetTexture(0, NULL);
   else
      m_Device->SetTexture(0, m_textureList[texId].image);
}


void CD3DRenderer::SaveScreenShot(char *file)
{
   if(!file) return;

   LPDIRECT3DSURFACE9 surface = NULL;
   D3DDISPLAYMODE disp;

   m_Device->GetDisplayMode(0, &disp);
   m_Device->CreateOffscreenPlainSurface(disp.Width, disp.Height,
      D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

   m_Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
   D3DXSaveSurfaceToFile(file, D3DXIFF_JPG, surface, NULL, NULL);

   if(surface != NULL) surface->Release();
   surface = NULL;
}


void CD3DRenderer::EnablePointSprites(float size, float min,
                                      float a, float b, float c)
{
   if(!m_Device) return;

   m_Device->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
   m_Device->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
   m_Device->SetRenderState(D3DRS_POINTSIZE, FtoDW(size));
   m_Device->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(min));
   m_Device->SetRenderState(D3DRS_POINTSCALE_A, FtoDW(a));
   m_Device->SetRenderState(D3DRS_POINTSCALE_B, FtoDW(b));
   m_Device->SetRenderState(D3DRS_POINTSCALE_C, FtoDW(c));
}


void CD3DRenderer::DisablePointSprites()
{
   m_Device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
   m_Device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
}


bool CD3DRenderer::CreateText(char *font, int weight, bool italic,
                              int size, int &id)
{
   if(!m_fonts)
      {
         m_fonts = new LPD3DXFONT[1];
         if(!m_fonts) return UGP_FAIL;
      }
   else
      {
         LPD3DXFONT *temp;
         temp = new LPD3DXFONT[m_totalFonts + 1];

         memcpy(temp, m_fonts,
                sizeof(LPD3DXFONT) * m_totalFonts);

         delete[] m_fonts;
         m_fonts = temp;
      }

   if(FAILED(D3DXCreateFont(m_Device, size, 0, weight, 1, italic,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            DEFAULT_QUALITY,
                            DEFAULT_PITCH | FF_DONTCARE, font,
                            &m_fonts[m_totalFonts]))) return false;

   id = m_totalFonts;
   m_totalFonts++;

   return true;
}


void CD3DRenderer::DisplayText(int id, long x, long y,
                               int r, int g, int b, char *text, ...)
{
   RECT FontPosition = {x, y, m_screenWidth, m_screenHeight};
   char message[1024];
   va_list argList;

   if(id >= m_totalFonts) return;
   
   va_start(argList, text);
   vsprintf(message, text, argList);
   va_end(argList);
   
   m_fonts[id]->DrawText(NULL, message, -1, &FontPosition,
         DT_SINGLELINE, D3DCOLOR_ARGB(255, r, g, b));
}


void CD3DRenderer::DisplayText(int id, long x, long y,
                               unsigned long color, char *text, ...)
{
   RECT FontPosition = {x, y, m_screenWidth, m_screenHeight};
   char message[1024];
   va_list argList;

   if(id >= m_totalFonts) return;
   
   va_start(argList, text);
   vsprintf(message, text, argList);
   va_end(argList);
   
   m_fonts[id]->DrawText(NULL, message, -1, &FontPosition,
                         DT_SINGLELINE, color);
}


bool CD3DRenderer::AddGUIBackdrop(int guiId, char *fileName)
{
   if(guiId >= m_totalGUIs) return false;

   int texID = -1, staticID = -1;

   if(!AddTexture2D(fileName, &texID)) return false;

   unsigned long col = D3DCOLOR_XRGB(255,255,255);

   stGUIVertex obj[] =
      {
         {(float)m_screenWidth, 0, 0, 1, col, 1, 0},
         {(float)m_screenWidth, (float)m_screenHeight,
           0, 1, col, 1, 1},
	      {0, 0, 0, 1, col, 0, 0},
	      {0, (float)m_screenHeight, 0, 1, col, 0, 1},
      };

   if(!CreateStaticBuffer(GUI_FVF, TRIANGLE_STRIP, 4, 0,
      sizeof(stGUIVertex), (void**)&obj, NULL,
      &staticID)) return false;

   return m_guiList[guiId].AddBackdrop(texID, staticID);
}


bool CD3DRenderer::AddGUIStaticText(int guiId, int id,
   char *text, int x, int y, unsigned long color, int fontID)
{
   if(guiId >= m_totalGUIs) return false;

   return m_guiList[guiId].AddStaticText(id, text, x, y,
                                         color, fontID);
}


bool CD3DRenderer::AddGUIButton(int guiId, int id, int x,
   int y, char *up, char *over, char *down)
{
   if(guiId >= m_totalGUIs) return false;

   int upID = -1, overID = -1, downID = -1, staticID = -1;

   if(!AddTexture2D(up, &upID)) return false;
   if(!AddTexture2D(over, &overID)) return false;
   if(!AddTexture2D(down, &downID)) return false;

   unsigned long col = D3DCOLOR_XRGB(255,255,255);

   int w = m_textureList[upID].width;
   int h = m_textureList[upID].height;

   stGUIVertex obj[] =
      {
         {(float)(w + x), (float)(0 + y), 0, 1, col, 1, 0},
         {(float)(w + x), (float)(h + y), 0, 1, col, 1, 1},
	      {(float)(0 + x), (float)(0 + y), 0, 1, col, 0, 0},
	      {(float)(0 + x), (float)(h + y), 0, 1, col, 0, 1},
      };

   if(!CreateStaticBuffer(GUI_FVF, TRIANGLE_STRIP, 4, 0,
      sizeof(stGUIVertex), (void**)&obj, NULL,
      &staticID)) return false;

   return m_guiList[guiId].AddButton(id, x, y, w, h, upID, overID,
                                     downID, staticID);
}


void CD3DRenderer::ProcessGUI(int guiId, bool LMBDown,
   int mouseX, int mouseY, void(*funcPtr)(int id, int state))
{
   if(guiId >= m_totalGUIs || !m_Device) return;

   CGUISystem *gui = &m_guiList[guiId];
   if(!gui) return;

   // Draw background.
   stGUIControl *backDrop = gui->GetBackDrop();

   // Draw backdrop first to control render order. Don't
   // want to draw this after buttons in 2D.
   if(backDrop)
      {
         ApplyTexture(0, backDrop->m_upTex);
         Render(backDrop->m_listID);
         ApplyTexture(0, -1);
      }

   // Initial button state.
   int status = UGP_BUTTON_UP;

   // Loop through all controls and display them.
   for(int i = 0; i < gui->GetTotalControls(); i++)
      {
         // Get the current control.
         stGUIControl *pCnt = gui->GetGUIControl(i);
         if(!pCnt) continue;

         // Take action depending on what type it is.
         switch(pCnt->m_type)
            {
               case UGP_GUI_STATICTEXT:
                  DisplayText(pCnt->m_listID, pCnt->m_xPos,
                     pCnt->m_yPos, pCnt->m_color, pCnt->m_text);
                  break;

               case UGP_GUI_BUTTON:
                  status = UGP_BUTTON_UP;

                  // Set alpha transparency on for the texture image.
                  m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE,
                                           TRUE);
                  m_Device->SetRenderState(D3DRS_SRCBLEND,
                                           D3DBLEND_SRCALPHA);
                  m_Device->SetRenderState(D3DRS_DESTBLEND,
                                           D3DBLEND_INVSRCALPHA);

                  // Check if over the button or pressing it.
                  if(mouseX > pCnt->m_xPos && mouseX <
                     (pCnt->m_xPos + pCnt->m_width) &&
                     mouseY > pCnt->m_yPos && mouseY <
                     (pCnt->m_yPos + pCnt->m_height))
                     {
                        if(LMBDown) status = UGP_BUTTON_DOWN;
                        else status = UGP_BUTTON_OVER;
                     }

                  // State will depend on it's texture.
                  if(status == UGP_BUTTON_UP)
                     ApplyTexture(0, pCnt->m_upTex);
                  if(status == UGP_BUTTON_OVER)
                     ApplyTexture(0, pCnt->m_overTex);
                  if(status == UGP_BUTTON_DOWN)
                     ApplyTexture(0, pCnt->m_downTex);

                  // Render button.
                  Render(pCnt->m_listID);

                  // Turn off alpha.
                  m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE,
                                           FALSE);
                  break;
            }

         // Process control by calling the callback.
         if(funcPtr) funcPtr(pCnt->m_id, status);
      }
}


void CD3DRenderer::CalculateProjMatrix(float fov, float n, float f)
{
   if(!m_Device) return;
   D3DXMATRIX projection;

   D3DXMatrixPerspectiveFovLH(&projection, fov,
      (float)m_screenWidth/(float)m_screenHeight, n, f);

   m_Device->SetTransform(D3DTS_PROJECTION, &projection);
}


void CD3DRenderer::CalculateOrthoMatrix(float n, float f)
{
   if(!m_Device) return;
   D3DXMATRIX ortho;

   D3DXMatrixOrthoLH(&ortho, (float)m_screenWidth,
                     (float)m_screenHeight, n, f);
   m_Device->SetTransform(D3DTS_PROJECTION, &ortho);
}


int CD3DRenderer::CreateStaticBuffer(VertexType vType,
   PrimType primType, int totalVerts,
   int totalIndices, int stride, void **data,
   unsigned int *indices, int *staticId)
{
   void *ptr;
   int index = m_numStaticBuffers;

   if(!m_staticBufferList)
      {
         m_staticBufferList = new stD3DStaticBuffer[1];
         if(!m_staticBufferList) return UGP_FAIL;
      }
   else
      {
         stD3DStaticBuffer *temp;
         temp = new stD3DStaticBuffer[m_numStaticBuffers + 1];

         memcpy(temp, m_staticBufferList,
                sizeof(stD3DStaticBuffer) * m_numStaticBuffers);

         delete[] m_staticBufferList;
         m_staticBufferList = temp;
      }

   m_staticBufferList[index].numVerts = totalVerts;
   m_staticBufferList[index].numIndices = totalIndices;
   m_staticBufferList[index].primType = primType;
   m_staticBufferList[index].stride = stride;
   m_staticBufferList[index].fvf = CreateD3DFVF(vType);
   
   if(totalIndices > 0)
      {
         if(FAILED(m_Device->CreateIndexBuffer(sizeof(unsigned int) *
                    totalIndices, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                    D3DPOOL_DEFAULT,
                    &m_staticBufferList[index].ibPtr,
                    NULL))) return UGP_FAIL;
         
         if(FAILED(m_staticBufferList[index].ibPtr->Lock(0, 0,
               (void**)&ptr, 0))) return UGP_FAIL;
         
         memcpy(ptr, indices, sizeof(unsigned int) * totalIndices);
         m_staticBufferList[index].ibPtr->Unlock();
      }
   else
      {
         m_staticBufferList[index].ibPtr = NULL;
      }
   
   if(FAILED(m_Device->CreateVertexBuffer(totalVerts * stride,
         D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf,
         D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr,
         NULL))) return UGP_FAIL;
   
   if(FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0,
             (void**)&ptr, 0))) return UGP_FAIL;
   
   memcpy(ptr, data, totalVerts * stride);
   m_staticBufferList[index].vbPtr->Unlock();
   
  
   *staticId = m_numStaticBuffers;
   m_numStaticBuffers++;
   
   return UGP_OK;
}


int CD3DRenderer::Render(int staticId)
{
   if(staticId >= m_numStaticBuffers) return UGP_FAIL;
   
   if(m_activeStaticBuffer != staticId)
      {
         if(m_staticBufferList[staticId].ibPtr != NULL)
            m_Device->SetIndices(m_staticBufferList[staticId].ibPtr);
         
         m_Device->SetStreamSource(0,
            m_staticBufferList[staticId].vbPtr, 0,
            m_staticBufferList[staticId].stride);

         m_Device->SetFVF(m_staticBufferList[staticId].fvf);

         m_activeStaticBuffer = staticId;
      }
   
   if(m_staticBufferList[staticId].ibPtr != NULL)
      {
         switch(m_staticBufferList[staticId].primType)
            {
               case POINT_LIST:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_POINTLIST,
                        0, m_staticBufferList[staticId].numVerts)))
                     return UGP_FAIL;
                  break;

               case TRIANGLE_LIST:
                  if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
                        0, m_staticBufferList[staticId].numVerts / 3,
                        0, m_staticBufferList[staticId].numIndices)))
                     return UGP_FAIL;
                  break;
               
               case TRIANGLE_STRIP:
                  if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0,
                        0, m_staticBufferList[staticId].numVerts / 2,
                        0, m_staticBufferList[staticId].numIndices)))
                     return UGP_FAIL;
                  break;
               
               case TRIANGLE_FAN:
                  if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0,
                        0, m_staticBufferList[staticId].numVerts / 2,
                        0, m_staticBufferList[staticId].numIndices)))
                     return UGP_FAIL;
                  break;

               case LINE_LIST:
                  if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_LINELIST, 0,
                        0, m_staticBufferList[staticId].numVerts / 2,
                        0, m_staticBufferList[staticId].numIndices)))
                     return UGP_FAIL;
                  break;
               
               case LINE_STRIP:
                  if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0,
                        0, m_staticBufferList[staticId].numVerts,
                        0, m_staticBufferList[staticId].numIndices)))
                     return UGP_FAIL;
                  break;

               default:
                  return UGP_FAIL;
            }
      }
   else
      {
         switch(m_staticBufferList[staticId].primType)
            {
               case POINT_LIST:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_POINTLIST,
                        0, m_staticBufferList[staticId].numVerts)))
                     return UGP_FAIL;
                  break;

               case TRIANGLE_LIST:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0,
                        (int)(m_staticBufferList[staticId].numVerts / 3))))
                     return UGP_FAIL;
                  break;
               
               case TRIANGLE_STRIP:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0,
                        (int)(m_staticBufferList[staticId].numVerts / 2))))
                     return UGP_FAIL;
                  break;
               
               case TRIANGLE_FAN:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0,
                        (int)(m_staticBufferList[staticId].numVerts / 2))))
                     return UGP_FAIL;
                  break;

               case LINE_LIST:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_LINELIST, 0,
                        m_staticBufferList[staticId].numVerts / 2)))
                     return UGP_FAIL;
                  break;
               
               case LINE_STRIP:
                  if(FAILED(m_Device->DrawPrimitive(D3DPT_LINESTRIP, 0,
                        m_staticBufferList[staticId].numVerts)))
                     return UGP_FAIL;
                  break;

               default:
                  return UGP_FAIL;
            }
      }
   
   return UGP_OK;
}