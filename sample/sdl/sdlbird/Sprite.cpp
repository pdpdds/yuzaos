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
#include <minwindef.h>
#include "Sprite.h"

CSprite::CSprite(SDL_Renderer *pRenderer, const char *szImageFileName, const char *szTxtFileName)
{
	Load(pRenderer, szImageFileName, szTxtFileName);
}

CSprite::~CSprite()
{
	if (m_pTexture != NULL)
	{
		SDL_DestroyTexture(m_pTexture);
	}
}

/**
 * This hash function has been taken from an Article in Dr Dobbs Journal.
 * This is normally a collision-free function, distributing keys evenly.
 * Collision can be avoided by comparing the key itself in last resort.
 */
inline unsigned int CalcTag(const char *sz)
{
	unsigned int hash = 0;

	while (*sz)
	{
		hash += (unsigned short)*sz;
		hash += (hash << 10);
		hash ^= (hash >> 6);

		sz++;
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

void CSprite::Draw(SDL_Renderer *pRenderer, const char *szTag, int x, int y)
{
	unsigned int uiTag = CalcTag(szTag);

	std::map<unsigned int, SpritePart_t>::iterator it = m_mapSpriteParts.find(uiTag);

	if (it != m_mapSpriteParts.end())
	{
		SDL_Rect srcrect, dstrect;

		srcrect.x = (int)it->second.X;
		srcrect.y = (int)it->second.Y;
		srcrect.w = (int)it->second.usWidth;
		srcrect.h = (int)it->second.usHeight;

		dstrect.x = (int)x;
		dstrect.y = (int)y;
		dstrect.w = (int)it->second.usWidth-1;
		dstrect.h = (int)it->second.usHeight-1;

		SDL_RenderCopy(pRenderer, m_pTexture, &srcrect, &dstrect);
	}
}

void CSprite::DrawEx(SDL_Renderer *pRenderer, const char *szTag, int x, int y, double angle, SDL_RendererFlip flip)
{
	unsigned int uiTag = CalcTag(szTag);

	std::map<unsigned int, SpritePart_t>::iterator it = m_mapSpriteParts.find(uiTag);

	if (it != m_mapSpriteParts.end())
	{
		SDL_Rect srcrect, dstrect;

		srcrect.x = it->second.X;
		srcrect.y = it->second.Y;
		srcrect.w = it->second.usWidth;
		srcrect.h = it->second.usHeight;

		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = it->second.usWidth;
		dstrect.h = it->second.usHeight;

		SDL_RenderCopyEx(pRenderer, m_pTexture, &srcrect, &dstrect, angle, NULL, flip);
	}
}

void CSprite::Load(SDL_Renderer *pRenderer, const char *szImageFileName, const char *szTxtFileName)
{
	SDL_Surface *pSurface = SDL_LoadBMP(szImageFileName);

	if (pSurface == NULL)
	{
		fprintf(stderr, "CSprite::Load(): IMG_Load failed: %s\n", SDL_GetError());
		return;
	}

	m_iTextureWidth = pSurface->w;
	m_iTextureHeight = pSurface->h;

	m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
	SDL_FreeSurface(pSurface);

	if (m_pTexture == NULL)
	{
		fprintf(stderr, "CSprite::Load(): SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		return;
	}

	// Load txt file
	if (!LoadTxt(szTxtFileName))
	{
		SDL_DestroyTexture(m_pTexture);
		m_pTexture = NULL;

		fprintf(stderr, "CSprite::Load(): LoadTxte failed\n");
		return;
	}
}

bool CSprite::LoadTxt(const char *szTxtFileName)
{
	SDL_RWops *rwops = SDL_RWFromFile(szTxtFileName, "r");

	if (rwops == NULL)
	{
		return false;
	}

	char *pBuf = (char *)malloc(SDL_RWsize(rwops) + 1);
	if (pBuf == NULL)
	{
		SDL_RWclose(rwops);
		return false;
	}

	SDL_RWread(rwops, pBuf, 1, SDL_RWsize(rwops));
	pBuf[SDL_RWsize(rwops)] = '\0';

	SDL_RWclose(rwops);

	char *p = pBuf;

	while (p != NULL && *p != '\0')
	{
		char name[256];
		int w, h;
		float x1, y1, x2, y2;

		if (sscanf(p, "%s %d %d %f %f %f %f", name, &w, &h, &x1, &y1, &x2, &y2) != 7)
		{
			p = strstr(p, "\n");
			if (p != NULL)
			{
				while (*p == '\n')
				{
					p++;
				}
			}
			continue;
		}

		p = strstr(p, "\n");
		if (p != NULL)
		{
			while (*p == '\n')
			{
				p++;
			}
		}

		SpritePart_t spritePart;

		spritePart.usWidth = w;
		spritePart.usHeight = h;
		spritePart.X = (unsigned short)(m_iTextureWidth * x1);
		spritePart.Y = (unsigned short)(m_iTextureHeight * y1);

		unsigned int uiTag = CalcTag(name);

		if (m_mapSpriteParts.find(uiTag) == m_mapSpriteParts.end())
		{
			m_mapSpriteParts[uiTag] = spritePart;
			m_mapSpriteParts[uiTag] = spritePart;
		}
		else
		{
			fprintf(stderr, "CSprite::LoadTxt(): WARNING, duplicate tag: %s %u\n", name, uiTag);
		}
	}

	free(pBuf);
	return true;
}
