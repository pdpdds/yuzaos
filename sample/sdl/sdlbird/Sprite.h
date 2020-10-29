/*
 * Copyright (c) 2014, Wei Mingzhi <whistler_wmz@users.sf.net>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author and contributors may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <SDL.h>

#include <map>

typedef struct tagSpritePart
{
  unsigned short usWidth;
  unsigned short usHeight;
  unsigned short X, Y;
} SpritePart_t;

class CSprite
{
 public:
  CSprite(SDL_Renderer *pRenderer, const char *szImageFileName, const char *szTxtFileName);
  virtual ~CSprite();

  void                  Draw(SDL_Renderer *pRenderer, const char *szTag, int x, int y);
  void                  DrawEx(SDL_Renderer *pRenderer, const char *szTag, int x, int y, double angle, SDL_RendererFlip flip);
  void                  SetAlpha(unsigned char alpha) { SDL_SetTextureAlphaMod(m_pTexture, alpha); }
  void                  SetColorMod(unsigned char r, unsigned char g, unsigned char b) { SDL_SetTextureColorMod(m_pTexture, r, g, b); }

 private:
  void                  Load(SDL_Renderer *pRenderer, const char *szImageFileName, const char *szTxtFileName);
  bool                  LoadTxt(const char *szTxtFileName);

  SDL_Texture          *m_pTexture;
  int                   m_iTextureWidth;
  int                   m_iTextureHeight;
  std::map<unsigned int, SpritePart_t>    m_mapSpriteParts;
};

#endif /* _SPRITE_H_ */








