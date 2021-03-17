#include "SGLObjModel.h"
#include "sgl.h"
#include <stdio.h>
SGLObjModel::SGLObjModel(void)
{
	m_bHasTexCoords = false;
	m_bHasNormals = false;
}
SGLObjModel::SGLObjModel(const char * filename)
{
	m_bHasTexCoords = false;
	m_bHasNormals = false;
	Load(filename);
}
SGLObjModel::~SGLObjModel(void)
{
}

bool SGLObjModel::Load(const char * filename)
{
	m_vVertices.clear();  //x, y, z 사용
	m_vNormals.clear();  //x, y, z 사용
	m_vTexCoords.clear(); //x, y만 사용
	m_vFaces.clear();

	char cLine[256];	

	FILE * fp = fopen(filename, "rb");
	if(!fp)
	{
		printf("Could not open %s", filename);
		return false;
	}

	while(!feof(fp))
	{
		int iStart = fgetc(fp);

		//If the first letter is v, it is either a vertex, a text coord, or a vertex normal
		if(iStart == 'v')
		{
			//get the second char
			int iNext = fgetc(fp);
			float fTemp[3];

			//if its a space, its a vertex coordinate
			if(iNext == ' ' || iNext == '\t')
			{
				//get the line
				fgets(cLine, 256, fp);
				//get the vertex coords
				sscanf(cLine, "%f %f %f", &fTemp[0], &fTemp[1], &fTemp[2]);
				//add to the vertex array
				m_vVertices.push_back(SGLPoint3D(fTemp[0], fTemp[1], fTemp[2]));
			}
			//if its a t, its a texture coord
			else if(iNext == 't')
			{
				//get the line
				fgets(cLine, 256, fp);
				//get the vertex coords
				sscanf(cLine, "%f %f", &fTemp[0], &fTemp[1]);
				//add to the vertex array
				m_vTexCoords.push_back(SGLPoint3D(fTemp[0], fTemp[1], 0));
				m_bHasTexCoords = true;
			}
			//if its an n its a normal
			else if(iNext == 'n')
			{
				//get the line
				fgets(cLine, 256, fp);
				//get the vertex coords
				sscanf(cLine, "%f %f %f", &fTemp[0], &fTemp[1], &fTemp[2]);
				//add to the vertex array
				m_vNormals.push_back(SGLVector3D(fTemp[0], fTemp[1], fTemp[2]));
				m_bHasNormals = true;
			}
			//else its something we don't support
			else
			{
				//scan the line and discard it
				fgets(cLine, 256, fp);
			}


		}
		//if the first letter is f, its a face
		else if(iStart == 'f')
		{
			//temp buffer to hold vertex indices
			int iTemp[3][3];
			memset(iTemp, 0, 36);
			//read in the line
			fgets(cLine, 256, fp);

			//If it has texture coords AND vertex normals
			if(m_bHasTexCoords && m_bHasNormals)
			{
				//extract the face info
				sscanf(cLine, "%i/%i/%i %i/%i/%i %i/%i/%i", &iTemp[0][0], &iTemp[1][0], &iTemp[2][0], 
															 &iTemp[0][1], &iTemp[1][1], &iTemp[2][1],
															 &iTemp[0][2], &iTemp[1][2], &iTemp[2][2]);
				//store the info in the faces structure
				m_vFaces.push_back(SGLObjFace(&iTemp[0][0]));
				
			}
			//Just has tex coords
			else if(m_bHasTexCoords && !m_bHasNormals)
			{
				//extract the face info
				sscanf(cLine, "%i/%i %i/%i %i/%i", &iTemp[0][0], &iTemp[1][0], 
													&iTemp[0][1], &iTemp[1][1], 
													&iTemp[0][2], &iTemp[1][2]);
				
				//store the info in the faces structure
				m_vFaces.push_back(&iTemp[0][0]);
			}
			//just normals
			else if(!m_bHasTexCoords && m_bHasNormals)
			{
				sscanf(cLine, "%i//%i %i//%i %i//%i", &iTemp[0][0], &iTemp[2][0], 
													&iTemp[0][1], &iTemp[2][1], 
													&iTemp[0][2], &iTemp[2][2]);
				//store the info in the faces structure
				m_vFaces.push_back(SGLObjFace(&iTemp[0][0]));

			}
			//Just vertices
			else
			{
				//extract the face info
				sscanf(cLine, "%i %i %i", &iTemp[0][0], &iTemp[0][1], &iTemp[0][2]);
				//store the info in the faces structure
				m_vFaces.push_back(SGLObjFace(&iTemp[0][0]));

			}			
			
		}
		//if it isn't any of those, we don't care about it
		else
		{
			//read the whole line to advance
			fgets(cLine, 256, fp);
		}
	}

	//m_pVerts = &m_vVertices[0];
	//m_pTexCoords = &m_vTexCoords[0];
	//m_pNormals = &m_vNormals[0];
	//m_pFaces = &m_vFaces[0];
	fclose(fp);

	return true;
}
void SGLObjModel::Render()
{
	static int iNumFaces = m_vFaces.size();

	sglBegin(SGL_TRIANGLES);
	for(int i = 0; i < iNumFaces; i++)
	{
		sglVertex3f(m_vVertices[(m_vFaces[i].m_uiVertIdx[0])].x, m_vVertices[(m_vFaces[i].m_uiVertIdx[0])].y, m_vVertices[(m_vFaces[i].m_uiVertIdx[0])].z);
		sglVertex3f(m_vVertices[(m_vFaces[i].m_uiVertIdx[1])].x, m_vVertices[(m_vFaces[i].m_uiVertIdx[1])].y, m_vVertices[(m_vFaces[i].m_uiVertIdx[1])].z);
		sglVertex3f(m_vVertices[(m_vFaces[i].m_uiVertIdx[2])].x, m_vVertices[(m_vFaces[i].m_uiVertIdx[2])].y, m_vVertices[(m_vFaces[i].m_uiVertIdx[2])].z);
	}
	sglEnd();
}


