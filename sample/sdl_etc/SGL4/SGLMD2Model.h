//-------------------------------------------------------------
// MD2 모델 파일 로더
// 데모 시연을 위해서 Focus On 3D Models 책의 코드를 사용하였음
//

#ifndef SGLMD2MODEL_H
#define SGLMD2MODEL_H

#include "timer.h"

struct SMD2Header
{
   int m_iMagicNum; //Always IDP2 (844121161)
   int m_iVersion;  //8
   int m_iSkinWidthPx;  
   int m_iSkinHeightPx; 
   int m_iFrameSize; 
   int m_iNumSkins; 
   int m_iNumVertices; 
   int m_iNumTexCoords; 
   int m_iNumTriangles; 
   int m_iNumGLCommands; 
   int m_iNumFrames; 
   int m_iOffsetSkins; 
   int m_iOffsetTexCoords; 
   int m_iOffsetTriangles; 
   int m_iOffsetFrames; 
   int m_iOffsetGlCommands; 
   int m_iFileSize; 
};

//-------------------------------------------------------------
//- SMD2Vert
//- Vertex structure for MD2
struct SMD2Vert
{
	float m_fVert[3];
	unsigned char m_ucReserved;
};


//-------------------------------------------------------------
//- SMD2Frame
//- Frame information for the model file 
struct SMD2Frame
{
	float m_fScale[3];
	float m_fTrans[3];
	char m_caName[16];
	SMD2Vert * m_pVerts;

	//Cleans up after itself
	SMD2Frame()
	{
		m_pVerts = 0;
	}

	~SMD2Frame()
	{
		if(m_pVerts)
			delete [] m_pVerts;
	}
};

//-------------------------------------------------------------
//- SMD2Tri
//- Triangle information for the MD2
struct SMD2Tri
{
	unsigned short m_sVertIndices[3];
	unsigned short m_sTexIndices[3];
};

//-------------------------------------------------------------
//- SMD2TexCoord
//- Texture coord information for the MD2
struct SMD2TexCoord
{
	float m_fTex[2];
};

//-------------------------------------------------------------
//- SMD2Skin
//- Name of a single skin in the md2 file
struct SMD2Skin
{
	char m_caSkin[64];	//filename
	//CImage m_Image;		//Image file ready for texturing
	int  m_skinTexId;
};

class SGLMD2Model
{
public:

	//Set skin to one of the files specified in the md2 files itself
	void SetSkin(unsigned int uiSkin);
	//Set skin to a different image
	void SetSkin(int skinId);

	//Load the file
	bool Load(const char * szFilename);
	
	//Render file at the initial position
	void Render();
	//Render the file at a certain frame
	void Render(unsigned int uiFrame);

	//Animate the md2 model (start and end frames of 0 and 0 will loop through the WHOLE model
	void Animate(float fSpeed, unsigned int& uiStartFrame, unsigned int uiEndFrame = 0, bool bLoop = true);

	//constructors/destructo
	SGLMD2Model();
	SGLMD2Model(const char * szFile);
	~SGLMD2Model();

private:
	
	CTimer m_Timer;
	//file header information
	SMD2Header m_Head; 
	//Frame information
	SMD2Frame * m_pFrames;
	//Triangles
	SMD2Tri * m_pTriangles;
	//Texure coords
	SMD2TexCoord * m_pTexCoords;
	//Skin files
	SMD2Skin * m_pSkins;
	//Interpolated vertices
	SMD2Vert * m_pVerts;
	//Current skin
	unsigned int m_uiSkin;
	//Using a custom skin?
	bool m_bIsCustomSkin;
	//The custom skin
	int  m_skinTexId;
};

#endif