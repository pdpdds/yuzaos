#include "SGLObject.h"
#include "SGLObjectList.h"
#include "SGLMatrix44.h"
#include "SGLGraphicsPipeline.h"
#include "SGLRaster.h"
#include "SGLVertex.h"
#include "SGLFaceSet.h"
#include "SGLState.h"
#include <math.h>
#include <algorithm>
namespace g
{
	SGLFaceSet	  faceset;
	SGLObjectList olist;	
	int			  objectType; //현재 오브젝트 타입
}
SGLVertex	  v[4]; //임시 버텍스
int	vc=0; //임시 버텍스 카운트
void sglBegin(int objectType)
{
	g::olist.addObject(SGLObject(objectType));
	g::objectType = objectType;
}
void sglSetTexture(int textureId)
{
	g::olist.getLastObject().textureIds.addIndex(textureId);
}
void sglVertices(const SGLVertex* vlist, int count)
{
	g::olist.getLastObject().indexedType = true;
	g::olist.getLastObject().localVertices.addVertex(vlist, count);
	g::olist.getLastObject().numVertices = count;
}
void sglVertex3f(float x, float y, float z)
{
	
	v[vc].pos.x = x;
	v[vc].pos.y = y;
	v[vc].pos.z = z;
	vc++;
	//g::olist.getLastObject().localVertices.addVertex(v[vc++]);
	//g::olist.getLastObject().numVertices++;
	g::olist.getLastObject().indexedType = false;
	if(vc == g::objectType)
	{
		if(g::objectType == SGL_TRIANGLES)
		{
			g::olist.getLastObject().vplist.addPolygon(SGLVertexPolygon(v[0], v[1], v[2]));
			g::olist.getLastObject().numVertices += 3;
		}
		else
		if(g::objectType == SGL_QUADS)
		{
			g::olist.getLastObject().vplist.addPolygon(SGLVertexPolygon(v[0], v[1], v[2], v[3]));
			g::olist.getLastObject().numVertices += 4;
		}
		vc = 0;
		g::olist.getLastObject().numPolygons++;
	}
}
void sglIndexes(const int* vilist, int count)
{
	int step = g::olist.getLastObject().objectType;
	g::olist.getLastObject().indexedType = true;
	for(int i=0; i<count; i+=step)
	{
		g::olist.getLastObject().plist.addPolygon(vilist+i, step);
	}
	g::olist.getLastObject().numPolygons = count / step;
}
void sglIndex3i(int v1, int v2, int v3)
{
	g::olist.getLastObject().indexedType = true;
	g::olist.getLastObject().plist.addPolygon(SGLPolygon(v1, v2, v3));
	g::olist.getLastObject().numPolygons++;
}
void sglIndex4i(int v1, int v2, int v3, int v4)
{
	g::olist.getLastObject().indexedType = true;
	g::olist.getLastObject().plist.addPolygon(SGLPolygon(v1, v2, v3, v4));
	g::olist.getLastObject().numPolygons++;
}
void sglColor3f(float r, float g, float b) //a = 1.0f
{
	v[vc].setColor(SGLColor(r*255, g*255, b*255));
}
void sglColor4f(float r, float g, float b, float a)
{
	v[vc].setColor(SGLColor(r*255, g*255, b*255, a*255));
}
void sglNormal3f(float x, float y, float z)
{
	v[vc].setNormal(SGLVector3D(x, y, z));
}
void sglTexCoord2f(float U, float V)
{
	v[vc].setTexCoord(U, V);
}
void sglCalcObjectRadius(SGLObject& obj)
{
	float radius;
	
	obj.radius = 0.0f;
	if(obj.indexedType)
	{
		for(int i=0; i<obj.numVertices; i++)
		{
			radius = obj.localVertices[i].pos.getLength();
			if(radius > obj.radius)
				obj.radius = radius;
		}
	}
	else
	{
		for(int i=0; i<obj.numPolygons; i++)
		{
			for(int j=0; j<obj.vplist[i].localVertices.size(); j++)
			{
				radius = obj.vplist[i].localVertices[j].pos.getLength();
				if(radius > obj.radius)
					obj.radius = radius;
			}
		}
	}
}
void sglObjectCulling(SGLObject& obj)
{
	//카메라 좌표 속에서의 위치를 구한다
	SGLPoint3D objectInC = sglGetCameraMatrix() * obj.worldPos;
	float x_bsphere,
		  y_bsphere,
		  z_bsphere,
		  radius,
		  x_compare,
		  y_compare;

	x_bsphere = objectInC.x;
	y_bsphere = objectInC.y;
	z_bsphere = objectInC.z;
	radius = obj.radius;

	if( ((z_bsphere-radius) > gp.clipZFar) ||
		((z_bsphere+radius) < gp.clipZNear) )
	{
		obj.visible = false;
		return ;
	}

	float halfView = (fabs(gp.clipXLeft) + fabs(gp.clipXRight))/2.0f;
	x_compare = (halfView*z_bsphere)/(gp.viewDistance);
	if( ((x_bsphere-radius) > x_compare) ||
		((x_bsphere+radius) < -x_compare) )
	{
		obj.visible = false;
		return ;
	}
	
	halfView = (fabs(gp.clipYTop) + fabs(gp.clipYBottom))/2.0f;
	y_compare = (halfView*z_bsphere)/(gp.viewDistance);
	if( ((y_bsphere-radius) > y_compare) ||
		((y_bsphere+radius) < -y_compare) )
	{
		obj.visible = false;
		return ;
	}
}
void sglBackfaceCulling(SGLObject& obj)
{
	SGLVector3D u, v, n, sight; 
	SGLPoint3D  v0, v1, v2;
	float d;
	float intencity;
	SGLColor lightColor;
	if(obj.indexedType)
	{
		for(int i=0; i<obj.numPolygons; i++)
		{
			v0 = obj.worldVertices[obj.plist[i].indexList[0]].pos;
			v1 = obj.worldVertices[obj.plist[i].indexList[1]].pos;
			v2 = obj.worldVertices[obj.plist[i].indexList[2]].pos;

			u = v0 - v1;
			v = v0 - v2;
			n = v.cross(u);
			n.normalize();
			//시선 계산
			//카메라시점 좌표에서 평면상의 임의점을 뺀다
			sight.x = gp.cameraEye.x - obj.worldVertices[obj.plist[i].indexList[0]].pos.x;
			sight.y = gp.cameraEye.y - obj.worldVertices[obj.plist[i].indexList[0]].pos.y;
			sight.z = gp.cameraEye.z - obj.worldVertices[obj.plist[i].indexList[0]].pos.z;
			d = n.dot(sight);
			if(d>0)
			{
				obj.plist[i].visible = true;
				if(sglGetState(SGL_LIGHT))
				{
					//빛이 표면을 반사하는지 검사해야 함
					//광원의 위치가 필요함
					d = n.dot(gp.light.diffuse);
					if(d>0)
					{
						lightColor = gp.light.color;
						lightColor.R *= gp.light.ambient.x;
						lightColor.G *= gp.light.ambient.y;
						lightColor.B *= gp.light.ambient.z;

						intencity = (n.dot(gp.light.diffuse) + 1.0f) / 2.0f;
						lightColor *= intencity;
						//obj.plist[i].intencity = lightColor;
						obj.localVertices[obj.plist[i].indexList[0]].color += lightColor;
						obj.localVertices[obj.plist[i].indexList[1]].color += lightColor;
						obj.localVertices[obj.plist[i].indexList[2]].color += lightColor;
						if(obj.objectType == SGL_QUADS)
						{
							obj.localVertices[obj.plist[i].indexList[3]].color += lightColor;
						}
					}
				}
			}
			else
			{
				obj.plist[i].visible = false;
			}
		}
	}
	else
	{
		for(int i=0; i<obj.numPolygons; i++)
		{
			v0 = obj.vplist[i].worldVertices[0].pos;
			v1 = obj.vplist[i].worldVertices[1].pos;
			v2 = obj.vplist[i].worldVertices[2].pos;

			u = v0 - v1;
			v = v0 - v2;
			n = v.cross(u);
			n.normalize();
			//시선 계산
			//카메라시점 좌표에서 평면상의 임의점을 뺀다
			sight.x = gp.cameraEye.x - obj.vplist[i].worldVertices[0].pos.x;
			sight.y = gp.cameraEye.y - obj.vplist[i].worldVertices[0].pos.y;
			sight.z = gp.cameraEye.z - obj.vplist[i].worldVertices[0].pos.z;
			d = n.dot(sight);
			if(d>0)
			{
				obj.vplist[i].visible = true;
				if(sglGetState(SGL_LIGHT))
				{
					//빛이 표면을 반사하는지 검사해야 함
					//광원의 위치가 필요함
					d = n.dot(gp.light.diffuse);
					if(d>0)
					{
						lightColor = gp.light.color;
						lightColor.R *= gp.light.ambient.x;
						lightColor.G *= gp.light.ambient.y;
						lightColor.B *= gp.light.ambient.z;

						intencity = (n.dot(gp.light.diffuse) + 1.0f) / 2.0f;
						lightColor *= intencity;
						
						obj.vplist[i].localVertices[0].color += lightColor;
						obj.vplist[i].localVertices[1].color += lightColor;
						obj.vplist[i].localVertices[2].color += lightColor;
						if(obj.objectType == SGL_QUADS)
						{
							obj.vplist[i].localVertices[3].color += lightColor;
						}
					}
				}
			}
			else
			{
				obj.vplist[i].visible = false;
			}
		}
	}
}
void sglClipInViewVolume(SGLObject& obj)
{
	
	SGLPoint3D v1, v2, v3, v4;
	float x1_compare,
		  y1_compare,
		  x2_compare,
		  y2_compare,
		  x3_compare,
		  y3_compare,
		  x4_compare,
		  y4_compare;

	float halfXView = (fabs(gp.clipXLeft) + fabs(gp.clipXRight))/2.0f;
	float halfYView = (fabs(gp.clipYTop) + fabs(gp.clipYBottom))/2.0f;

	if(obj.indexedType)
	{
		for(int i=0; i<obj.numPolygons; i++)
		{
			v1 = obj.cameraVertices[obj.plist[i].indexList[0]].pos;
			v2 = obj.cameraVertices[obj.plist[i].indexList[1]].pos;
			v3 = obj.cameraVertices[obj.plist[i].indexList[2]].pos;
			if(obj.objectType == SGL_QUADS)
			{
				v4 = obj.cameraVertices[obj.plist[i].indexList[3]].pos;

				//Z클리핑
				if( !((v1.z>gp.clipZNear || v2.z>gp.clipZNear || v3.z>gp.clipZNear || v4.z>gp.clipZNear) &&
					(v1.z<gp.clipZFar || v2.z<gp.clipZFar || v3.z<gp.clipZFar || v4.z<gp.clipZFar)))
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}
				
				x1_compare = (v1.z*halfXView)/gp.viewDistance;
				x2_compare = (v2.z*halfXView)/gp.viewDistance;
				x3_compare = (v3.z*halfXView)/gp.viewDistance;
				x4_compare = (v4.z*halfXView)/gp.viewDistance;

				if( !((v1.x>-x1_compare || v2.x>-x2_compare || v3.x>-x3_compare || v4.x>-x4_compare) &&
					(v1.x<x1_compare || v2.x<x2_compare || v3.x<x3_compare || v4.x<x4_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}

				y1_compare = (v1.z*halfYView)/gp.viewDistance;
				y2_compare = (v2.z*halfYView)/gp.viewDistance;
				y3_compare = (v3.z*halfYView)/gp.viewDistance;
				y4_compare = (v4.z*halfYView)/gp.viewDistance;

				if( !((v1.y>-y1_compare || v2.y>-y2_compare || v3.y>-y3_compare || v4.y>-y4_compare) &&
					(v1.y<y1_compare || v2.y<y2_compare || v3.y<y3_compare || v4.y<y4_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}
				
			}
			else
			if(obj.objectType == SGL_TRIANGLES)
			{
				//Z클리핑
				if( !((v1.z>gp.clipZNear || v2.z>gp.clipZNear || v3.z>gp.clipZNear) &&
					(v1.z<gp.clipZFar || v2.z<gp.clipZFar || v3.z<gp.clipZFar)))
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}
				
				x1_compare = (v1.z*halfXView)/gp.viewDistance;
				x2_compare = (v2.z*halfXView)/gp.viewDistance;
				x3_compare = (v3.z*halfXView)/gp.viewDistance;

				if( !((v1.x>-x1_compare || v2.x>-x2_compare || v3.x>-x3_compare) &&
					(v1.x<x1_compare || v2.x<x2_compare || v3.x<x3_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}

				y1_compare = (v1.z*halfYView)/gp.viewDistance;
				y2_compare = (v2.z*halfYView)/gp.viewDistance;
				y3_compare = (v3.z*halfYView)/gp.viewDistance;

				if( !((v1.y>-y1_compare || v2.y>-y2_compare || v3.y>-y3_compare) &&
					(v1.y<y1_compare || v2.y<y2_compare || v3.y<y3_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}
				
			}
			
		}
	}
	else //정점으로 정의된 것이라면
	{
		for(int i=0; i<obj.numPolygons; i++)
		{
			v1 = obj.vplist[i].cameraVertices[0].pos;
			v2 = obj.vplist[i].cameraVertices[1].pos;
			v3 = obj.vplist[i].cameraVertices[2].pos;
			if(obj.objectType == SGL_QUADS)
			{
				v4 = obj.vplist[i].cameraVertices[3].pos;

				//Z클리핑
				if( !((v1.z>gp.clipZNear || v2.z>gp.clipZNear || v3.z>gp.clipZNear || v4.z>gp.clipZNear) &&
					(v1.z<gp.clipZFar || v2.z<gp.clipZFar || v3.z<gp.clipZFar || v4.z<gp.clipZFar)))
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}

				x1_compare = (v1.z*halfXView)/gp.viewDistance;
				x2_compare = (v2.z*halfXView)/gp.viewDistance;
				x3_compare = (v3.z*halfXView)/gp.viewDistance;
				x4_compare = (v4.z*halfXView)/gp.viewDistance;

				if( !((v1.x>-x1_compare || v2.x>-x2_compare || v3.x>-x3_compare || v4.x>-x4_compare) &&
					(v1.x<x1_compare || v2.x<x2_compare || v3.x<x3_compare || v4.x<x4_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}

				y1_compare = (v1.z*halfYView)/gp.viewDistance;
				y2_compare = (v2.z*halfYView)/gp.viewDistance;
				y3_compare = (v3.z*halfYView)/gp.viewDistance;
				y4_compare = (v4.z*halfYView)/gp.viewDistance;

				if( !((v1.y>-y1_compare || v2.y>-y2_compare || v3.y>-y3_compare || v4.y>-y4_compare) &&
					(v1.y<y1_compare || v2.y<y2_compare || v3.y<y3_compare || v4.y<y4_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}
			}
			else
			if(obj.objectType == SGL_TRIANGLES)
			{
				//Z클리핑
				if( !((v1.z>gp.clipZNear || v2.z>gp.clipZNear || v3.z>gp.clipZNear) &&
					(v1.z<gp.clipZFar || v2.z<gp.clipZFar || v3.z<gp.clipZFar)))
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}

				x1_compare = (v1.z*halfXView)/gp.viewDistance;
				x2_compare = (v2.z*halfXView)/gp.viewDistance;
				x3_compare = (v3.z*halfXView)/gp.viewDistance;

				if( !((v1.x>-x1_compare || v2.x>-x2_compare || v3.x>-x3_compare) &&
					(v1.x<x1_compare || v2.x<x2_compare || v3.x<x3_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}

				y1_compare = (v1.z*halfYView)/gp.viewDistance;
				y2_compare = (v2.z*halfYView)/gp.viewDistance;
				y3_compare = (v3.z*halfYView)/gp.viewDistance;

				if( !((v1.y>-y1_compare || v2.y>-y2_compare || v3.y>-y3_compare) &&
					(v1.y<y1_compare || v2.y<y2_compare || v3.y<y3_compare)) )
				{
					obj.plist[i].clipped = true; //그려지지 않는다.
					continue;
				}
			}
			
		}
	}
}
void sglEnd()
{
	//변환
	SGLMatrix44 Mso = sglGetViewportMatrix() * sglGetProjectionMatrix() * sglGetCameraMatrix() * sglGetModelViewMatrix();
	SGLMatrix44 Mcw = sglGetCameraMatrix();
	SGLMatrix44 Mwo = sglGetModelViewMatrix();
	SGLObject& obj = g::olist.getLastObject();
	obj.worldPos.x = Mwo.m14;
	obj.worldPos.y = Mwo.m24;
	obj.worldPos.z = Mwo.m34;
	//if(sglGetState(SGL_OBJECT_CULL))
	//{
		//오브젝트 컬링을 위해서 제일 큰 반지름을 계산한다.
		sglCalcObjectRadius(obj); 
		//오브젝트 컬링을 한다.
		sglObjectCulling(obj);
		//보이지 않는 오브젝트이면 아무것도 하지 않는다.
		if(!obj.visible)
			return ;
	//}
	//정점 변환
	if(obj.indexedType) //인덱스로 정의된 오브젝트
	{
		for(int j=0; j<obj.localVertices.size(); j++)
		{
			obj.worldVertices.addVertex(SGLVertex(Mwo*obj.localVertices[j].pos));
		}	
		//보이지 않는 면을 추려낸다.
		//if(sglGetState(SGL_BACKFACE_CULL)) 
			sglBackfaceCulling(obj);

		for(int j=0; j<obj.worldVertices.size(); j++)
		{
			obj.cameraVertices.addVertex(SGLVertex(Mcw*obj.worldVertices[j].pos));
		}
		//최종 그려질 폴리곤 추려내기
		//if(sglGetState(SGL_CLIP_3D))
			sglClipInViewVolume(obj);
		
		SGLFace f;
		f.obj = &obj;
		for(int j=0; j<obj.numPolygons; j++)
		{
			if(obj.plist[j].visible && !obj.plist[j].clipped)
			{
				f.v[0] = obj.localVertices[obj.plist[j].indexList[0]];
				f.v[1] = obj.localVertices[obj.plist[j].indexList[1]];
				f.v[2] = obj.localVertices[obj.plist[j].indexList[2]];

				f.v[0].pos = Mso * obj.localVertices[obj.plist[j].indexList[0]].pos;
				f.v[1].pos = Mso * obj.localVertices[obj.plist[j].indexList[1]].pos;
				f.v[2].pos = Mso * obj.localVertices[obj.plist[j].indexList[2]].pos;
				if(obj.objectType == SGL_QUADS)
				{
					f.v[3] = obj.localVertices[obj.plist[j].indexList[3]];
					f.v[3].pos = Mso * obj.localVertices[obj.plist[j].indexList[3]].pos;
					f.depthZ = 0.25f * (f.v[0].pos.z + f.v[1].pos.z + f.v[2].pos.z + f.v[3].pos.z);
					f.numPoints = 4;
				}
				else
				{
					f.numPoints = 3;
					f.depthZ = 0.3333f * (f.v[0].pos.z + f.v[1].pos.z + f.v[2].pos.z);
				}
				g::faceset.addFace(f);
			}
		}
		
	}
	else //정점으로 정의된 오브젝트
	{
		for(int i=0; i<obj.numPolygons; i++)
		{
			for(int j=0; j<obj.vplist[i].localVertices.size(); j++)
			{
				obj.vplist[i].worldVertices.addVertex(SGLVertex(Mwo*obj.vplist[i].localVertices[j].pos));
			}
		}
		//보이지 않는 면을 추려낸다.
		//if(sglGetState(SGL_BACKFACE_CULL)) 
			sglBackfaceCulling(obj);


		for(int i=0; i<obj.numPolygons; i++)
		{
			for(int j=0; j<obj.vplist[i].worldVertices.size(); j++)
			{
				obj.vplist[i].cameraVertices.addVertex(SGLVertex(Mcw*obj.vplist[i].worldVertices[j].pos));
			}
		}
		
		//최종 그려질 폴리곤 추려내기
		//if(sglGetState(SGL_CLIP_3D))
			sglClipInViewVolume(obj);

		//최종 그려질 폴리곤 추려내기
		SGLFace f;
		f.obj = &obj;
		for(int i=0; i<obj.numPolygons; i++)
		{
			
			if(obj.vplist[i].visible && !obj.vplist[i].clipped)
			{
				f.v[0] = obj.vplist[i].localVertices[0];
				f.v[1] = obj.vplist[i].localVertices[1];
				f.v[2] = obj.vplist[i].localVertices[2];
				
				f.v[0].pos = Mso * obj.vplist[i].localVertices[0].pos;
				f.v[1].pos = Mso * obj.vplist[i].localVertices[1].pos;
				f.v[2].pos = Mso * obj.vplist[i].localVertices[2].pos;
				if(obj.objectType == SGL_QUADS)
				{
					f.v[3] = obj.vplist[i].localVertices[3];
					f.v[3].pos = Mso * obj.vplist[i].localVertices[3].pos;
					f.depthZ = 0.25f * (f.v[0].pos.z + f.v[1].pos.z + f.v[2].pos.z + f.v[3].pos.z);
					f.numPoints = 4;
				}
				else
				{
					f.depthZ = 0.3333f * (f.v[0].pos.z + f.v[1].pos.z + f.v[2].pos.z);
					f.numPoints = 3;
				}
				g::faceset.addFace(f);
			}
		}
	}
}
bool sglZDepthSort(const SGLFace& f1, const SGLFace& f2)
{
	return f1.depthZ < f2.depthZ;
}
//그리기를 수행한다
//모든 리스트를 초기화 한다
void sglSwapBuffer(void)
{
	//DepthSort 수행
	std::sort(g::faceset.faceset.begin(), g::faceset.faceset.end(), sglZDepthSort);
	sglDrawFaceSet(g::faceset);
	g::olist.clear();
	g::faceset.clear();
	vc = 0;
}
