#include "SGLPolygon.h"

SGLPolygon::SGLPolygon(int v1, int v2, int v3)
{
	indexList.vilist.push_back(v1);
	indexList.vilist.push_back(v2);
	indexList.vilist.push_back(v3);
	numVertex = 3;
	visible = true;	
	clipped = false;
}
SGLPolygon::SGLPolygon(int v1, int v2, int v3, int v4)
{
	indexList.vilist.push_back(v1);
	indexList.vilist.push_back(v2);
	indexList.vilist.push_back(v3);
	indexList.vilist.push_back(v4);
	numVertex = 4;
	visible = true;	
	clipped = false;
}

SGLVertexPolygon::SGLVertexPolygon(const SGLVertex& v1, const SGLVertex& v2, const SGLVertex& v3)
{
	localVertices.addVertex(v1);
	localVertices.addVertex(v2);
	localVertices.addVertex(v3);
	numVertex = 3;
	visible = true;	
	clipped = false;
}
SGLVertexPolygon::SGLVertexPolygon(const SGLVertex& v1, const SGLVertex& v2, const SGLVertex& v3, const SGLVertex& v4)
{
	localVertices.addVertex(v1);
	localVertices.addVertex(v2);
	localVertices.addVertex(v3);
	localVertices.addVertex(v4);
	numVertex = 4;
	visible = true;	
	clipped = false;
}