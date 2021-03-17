//-------------------------------------------------------------
// OBJ 모델 파일 로더
// 데모 시연을 위해서 Focus On 3D Models 책의 코드를 사용하였음
//
#ifndef SGLOBJMODEL_H
#define SGLOBJMODEL_H

#include <vector>
#include "SGLPoint3D.h"
#include "SGLVector3D.h"

struct SGLObjFace
{
	unsigned int m_uiVertIdx[3];
	unsigned int m_uiTexCoordIdx[3];
	unsigned int m_uiNormalIdx[3];

	SGLObjFace(int * ipData)
	{
		//copy the data;
		m_uiVertIdx[0] = ipData[0]-1;
		m_uiVertIdx[1] = ipData[1]-1;
		m_uiVertIdx[2] = ipData[2]-1;

		m_uiTexCoordIdx[0] = ipData[3]-1;
		m_uiTexCoordIdx[1] = ipData[4]-1;
		m_uiTexCoordIdx[2] = ipData[5]-1;

		m_uiNormalIdx[0] = ipData[6]-1;
		m_uiNormalIdx[1] = ipData[7]-1;
		m_uiNormalIdx[2] = ipData[8]-1;
	}
};

class SGLObjModel
{
public:
	bool Load(const char * filename);
	void Render();
	SGLObjModel();
	SGLObjModel(const char * filename);
	~SGLObjModel(void);
private:
	std::vector<SGLPoint3D> m_vVertices;  //x, y, z 사용
	std::vector<SGLVector3D> m_vNormals;  //x, y, z 사용
	std::vector<SGLPoint3D> m_vTexCoords; //x, y만 사용
	std::vector<SGLObjFace> m_vFaces;
	bool m_bHasNormals;
	bool m_bHasTexCoords;
};

#endif
