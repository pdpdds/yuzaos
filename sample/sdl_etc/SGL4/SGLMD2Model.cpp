#include "SGLMD2Model.h"
/*************************************************************/
/*                          MD2.CPP                          */
/*                                                           */
/* Purpose: Implementation for loader and animator of MD2    */
/*          3d models.                                       */
/*      Evan Pipho (evan@codershq.com)                       */
/*                                                           */
/*************************************************************/
//-------------------------------------------------------------
//                       INCLUDES                             -
//-------------------------------------------------------------
#include "SGLMD2Model.h"
#include "sgl.h"
#include <stdio.h>
//-------------------------------------------------------------
//- Load
//- Loads an MD2 model from file
//-------------------------------------------------------------
bool SGLMD2Model::Load(const char * szFilename)
{
	unsigned char * ucpBuffer = 0;
	unsigned char * ucpPtr = 0;
	unsigned char * ucpTmpPtr = 0; 
	int iFileSize = 0;
	FILE * f;
	
	if(!(f = fopen(szFilename, "rb")))
	{
		printf("Could not open MD2 file %s", szFilename);
		return false;
	}

	//check file size and read it all into the buffer
	int iStart = ftell(f);
	fseek(f, 0, SEEK_END);
	int iEnd = ftell(f);
	fseek(f, 0, SEEK_SET);
	iFileSize = iEnd - iStart;

	//Allocate memory for whole file
	ucpBuffer = new unsigned char[iFileSize];
	ucpPtr = ucpBuffer;

	if(!ucpBuffer)
	{
		printf("Could not allocate memory for %s", szFilename);
		return false;
	}

	//Load file into buffer
	if(fread(ucpBuffer, 1, iFileSize, f) != (unsigned)iFileSize)
	{
		printf("Could not read from %s", szFilename);
		delete [] ucpBuffer;
		return false;
	}

	//close the file, we don't need it anymore
	fclose(f);

	//get the header
	memcpy(&m_Head, ucpPtr, sizeof(SMD2Header));

	//make sure it is a valid MD2 file before we get going
	if(m_Head.m_iMagicNum != 844121161 || m_Head.m_iVersion != 8)
	{
		printf("%s is not a valid MD2 file", szFilename);
		delete [] ucpBuffer;
		return false;
	}
	
	ucpTmpPtr = ucpPtr;
	ucpTmpPtr += m_Head.m_iOffsetFrames;

	//read the frames
	m_pFrames = new SMD2Frame[m_Head.m_iNumFrames];
	
	int i;
	for(i = 0; i < m_Head.m_iNumFrames; i++)
	{
		float fScale[3];
		float fTrans[3];
		m_pFrames[i].m_pVerts = new SMD2Vert[m_Head.m_iNumVertices];
		//expand the verices
		memcpy(fScale, ucpTmpPtr, 12);
		memcpy(fTrans, ucpTmpPtr + 12, 12);
		memcpy(m_pFrames[i].m_caName, ucpTmpPtr + 24, 16);
		ucpTmpPtr += 40;
		for(int j = 0; j < m_Head.m_iNumVertices; j++)
		{
			//swap y and z coords to convert to the proper orientation on screen
			m_pFrames[i].m_pVerts[j].m_fVert[0] = ucpTmpPtr[0] * fScale[0] + fTrans[0];
			m_pFrames[i].m_pVerts[j].m_fVert[1] = ucpTmpPtr[2] * fScale[2] + fTrans[2];
			m_pFrames[i].m_pVerts[j].m_fVert[2] = ucpTmpPtr[1] * fScale[1] + fTrans[1];
			m_pFrames[i].m_pVerts[j].m_ucReserved = ucpTmpPtr[3];
			ucpTmpPtr += 4;
		}
		
	}

	//Read in the triangles
	ucpTmpPtr = ucpPtr;
	ucpTmpPtr += m_Head.m_iOffsetTriangles;
	m_pTriangles = new SMD2Tri[m_Head.m_iNumTriangles];
	memcpy(m_pTriangles, ucpTmpPtr, 12 * m_Head.m_iNumTriangles);

	//Read the U/V texture coords
	ucpTmpPtr = ucpPtr;
	ucpTmpPtr += m_Head.m_iOffsetTexCoords;
	m_pTexCoords = new SMD2TexCoord[m_Head.m_iNumTexCoords];
	
	short * sTexCoords = new short[m_Head.m_iNumTexCoords * 2];
	memcpy(sTexCoords, ucpTmpPtr, 4 * m_Head.m_iNumTexCoords);

	for(i = 0; i < m_Head.m_iNumTexCoords; i++)
	{
		m_pTexCoords[i].m_fTex[0] = (float)sTexCoords[2*i] / m_Head.m_iSkinWidthPx;
		m_pTexCoords[i].m_fTex[1] = (float)sTexCoords[2*i+1] / m_Head.m_iSkinHeightPx;
	}
	
	delete [] sTexCoords;

	//Read the skin filenames
	ucpTmpPtr = ucpPtr;
	ucpTmpPtr += m_Head.m_iOffsetSkins;
	m_pSkins = new SMD2Skin[m_Head.m_iNumSkins];
	
	//Load textures
	for(i = 0; i < m_Head.m_iNumSkins; i++)
	{
		memcpy(m_pSkins[i].m_caSkin, ucpTmpPtr, 64);
		//hack off the leading parts and just get the filename
		const char * szEnd = strrchr((const char*)m_pSkins[i].m_caSkin, '/');
		
		if(szEnd)
		{
			szEnd++;
			strcpy(m_pSkins[i].m_caSkin, szEnd);
		}

		//m_pSkins[i].m_Image.Load(m_pSkins[i].m_caSkin);
		m_pSkins[i].m_skinTexId = sglLoadTextureFromFile(m_pSkins[i].m_caSkin);
		ucpTmpPtr += 64;
	}
		
	delete [] ucpBuffer;
	return true;
}

//-------------------------------------------------------------
//- Render
//- Renders the model in its initial position (frame 0)
//-------------------------------------------------------------
void SGLMD2Model::Render()
{
	sglEnable(SGL_TEXTURE);
	sglBegin(SGL_TRIANGLES);
	if(!m_bIsCustomSkin)
		sglSetTexture(m_pSkins[m_uiSkin].m_skinTexId);
	else
		sglSetTexture(m_skinTexId);

	for(int x = 0; x < m_Head.m_iNumTriangles; x++)
	{
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[0]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[0]].m_fTex[1]);
		sglVertex3f(m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[0], m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[1], m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[2]);

		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[1]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[1]].m_fTex[1]);
		sglVertex3f(m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[0], m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[1], m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[2]);

		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[2]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[2]].m_fTex[1]);
		sglVertex3f(m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[0], m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[1], m_pFrames[0].m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[2]);
	
	}
	sglEnd();
}

//-------------------------------------------------------------
//- Render
//- Renders a specific frame of the MD2 model
//-------------------------------------------------------------
void SGLMD2Model::Render(unsigned int uiFrame)
{
	sglEnable(SGL_TEXTURE);
	



	sglBegin(SGL_TRIANGLES);
	if(!m_bIsCustomSkin)
		sglSetTexture(m_pSkins[m_uiSkin].m_skinTexId);
	else
		sglSetTexture(m_skinTexId);

	for(int x = 0; x < m_Head.m_iNumTriangles; x++)
	{
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[0]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[0]].m_fTex[1]);
		sglVertex3f(m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[0], m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[1], m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[2]);
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[1]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[1]].m_fTex[1]);
		sglVertex3f(m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[0], m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[1], m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[2]);
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[2]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[2]].m_fTex[1]);
		sglVertex3f(m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[0], m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[1], m_pFrames[uiFrame].m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[2]);
	}
	sglEnd();
}

//-------------------------------------------------------------
//- Animate 
//- Animates the MD2 file  fspeed is frames per second
//-------------------------------------------------------------
void SGLMD2Model::Animate(float fSpeed, unsigned int& uiStartFrame, unsigned int uiEndFrame, bool bLoop)
{
	static unsigned int uiTotalFrames = 0;			//total number of frames
	static unsigned int uiLastStart = 0, uiLastEnd = 0;	//last start/end parems passed to the function
	static unsigned int uiLastFrame = 0;			//lastframe rendered
	static unsigned int uiMSPerFrame = 0;			//number of milliseconds per frame
	static float fLastInterp = 0;					//Last interpolation value

	//alloc a place to put the interpolated vertices
	if(!m_pVerts)
		m_pVerts = new SMD2Vert[m_Head.m_iNumVertices];

	//Prevent invalid frame counts
	if(uiEndFrame >= (unsigned)m_Head.m_iNumFrames)
		uiEndFrame = m_Head.m_iNumFrames - 1;
	if(uiStartFrame >= (unsigned)m_Head.m_iNumFrames-1)
		uiStartFrame = 0;

	//avoid calculating everything every frame
	if(uiLastStart != uiStartFrame || uiLastEnd != uiEndFrame)
	{
		uiLastStart = uiStartFrame;
		uiLastEnd = uiEndFrame;
		if(uiStartFrame > uiEndFrame)
		{
			uiTotalFrames = m_Head.m_iNumFrames - uiStartFrame + uiEndFrame + 1;
		}
		else
		{
			uiTotalFrames = uiEndFrame - uiStartFrame;
		}
	}
	uiMSPerFrame = (unsigned int)(1000 / fSpeed);
	
	//Calculate the next frame and the interpolation value
	unsigned int uiTime = m_Timer.GetMS();
	float fInterpValue = ((float) uiTime / uiMSPerFrame) + fLastInterp;
	fLastInterp = fInterpValue;

	//If the interpolation value is greater than 1, we must increment the frame counter
	while(fInterpValue > 1.0f)
	{
		uiLastFrame ++;
		if(uiLastFrame >= uiEndFrame)
		{
			uiLastFrame = uiStartFrame;
		}
		fInterpValue -= 1.0f;
		fLastInterp = 0.0f;
	}

	SMD2Frame* pCurFrame = &m_pFrames[uiLastFrame];
	SMD2Frame* pNextFrame = &m_pFrames[uiLastFrame+1];

	if(uiLastFrame == uiEndFrame-1)
		pNextFrame = &m_pFrames[uiStartFrame];
	
//
	//interpolate the vertices
	int x;
	for(x = 0; x < m_Head.m_iNumVertices; x++)
	{
		m_pVerts[x].m_fVert[0] = pCurFrame->m_pVerts[x].m_fVert[0] + (pNextFrame->m_pVerts[x].m_fVert[0] - pCurFrame->m_pVerts[x].m_fVert[0]) * fInterpValue;
		m_pVerts[x].m_fVert[1] = pCurFrame->m_pVerts[x].m_fVert[1] + (pNextFrame->m_pVerts[x].m_fVert[1] - pCurFrame->m_pVerts[x].m_fVert[1]) * fInterpValue;
		m_pVerts[x].m_fVert[2] = pCurFrame->m_pVerts[x].m_fVert[2] + (pNextFrame->m_pVerts[x].m_fVert[2] - pCurFrame->m_pVerts[x].m_fVert[2]) * fInterpValue;
	}

	//Render the new vertices
	

	sglBegin(SGL_TRIANGLES);
	sglEnable(SGL_TEXTURE);
	if(!m_bIsCustomSkin)
		sglSetTexture(m_pSkins[m_uiSkin].m_skinTexId);
	else
		sglSetTexture(m_skinTexId);

	for(x = 0; x < m_Head.m_iNumTriangles; x++)
	{
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[0]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[0]].m_fTex[1]);
		sglVertex3f(m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[0], m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[1], m_pVerts[m_pTriangles[x].m_sVertIndices[0]].m_fVert[2]);
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[1]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[1]].m_fTex[1]);
		sglVertex3f(m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[0], m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[1], m_pVerts[m_pTriangles[x].m_sVertIndices[1]].m_fVert[2]);
		sglTexCoord2f(m_pTexCoords[m_pTriangles[x].m_sTexIndices[2]].m_fTex[0], m_pTexCoords[m_pTriangles[x].m_sTexIndices[2]].m_fTex[1]);
		sglVertex3f(m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[0],m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[1], m_pVerts[m_pTriangles[x].m_sVertIndices[2]].m_fVert[2]);
	}

	sglEnd();

	//Print debug info
	//printf("Frame: %i : %s", uiLastFrame, m_pFrames[uiLastFrame].m_caName);
}

//-------------------------------------------------------------
//- SetSkin
//- Sets the current skin to one of the skins predefined by the md2 itself
//-------------------------------------------------------------
void SGLMD2Model::SetSkin(unsigned int uiSkin)
{
	m_uiSkin = uiSkin;
	m_bIsCustomSkin = false;
}

//-------------------------------------------------------------
//- SetSkin
//- Sets the skin to an image loaded elsewhere using a CIMAGE object
//-------------------------------------------------------------
void SGLMD2Model::SetSkin(int skinId)
{
	//m_pCustSkin = &skin;
	this->m_skinTexId = skinId;
	m_bIsCustomSkin = true;
}

//-------------------------------------------------------------
//- Constructors
//- 1. Default Constructor
//- 2. takes filename, calls load
//-------------------------------------------------------------
SGLMD2Model::SGLMD2Model()
{
	m_pFrames = 0;
	m_pTriangles = 0;
	m_pTexCoords = 0;
	m_pSkins = 0;
	m_pVerts = 0;
	m_uiSkin = 0;
	m_bIsCustomSkin = false;
	//m_pCustSkin = 0;
	m_Timer.Init();
}

SGLMD2Model::SGLMD2Model(const char * szFile)
{
	m_pFrames = 0;
	m_pTriangles = 0;
	m_pTexCoords = 0;
	m_pSkins = 0;
	m_bIsCustomSkin = false;
	//m_pCustSkin = 0;
	m_pVerts = 0;
	m_uiSkin = 0;
	Load(szFile);
	m_Timer.Init();
}

SGLMD2Model::~SGLMD2Model()
{
	if(m_pFrames)
	{
		delete [] m_pFrames;
		m_pFrames = 0;
	}
	if(m_pTexCoords)
	{
		delete [] m_pTexCoords;
		m_pTexCoords = 0;
	}
	if(m_pTriangles)
	{
		delete [] m_pTriangles;
		m_pTriangles = 0;
	}
	if(m_pSkins)
	{
		delete [] m_pSkins;
		m_pSkins = 0;
	}
	if(m_pVerts)
	{
		delete[] m_pVerts;
		m_pVerts = 0;
	}
}