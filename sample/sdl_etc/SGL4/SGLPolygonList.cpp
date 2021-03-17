#include "SGLPolygonList.h"

SGLPolygonList::SGLPolygonList(void)
{
	clear();
	plist.reserve(1024);
}

SGLPolygonList::~SGLPolygonList(void)
{
	clear();
}
void SGLPolygonList::clear(void)
{
	plist.clear();
}
void SGLPolygonList::addPolygon(const SGLPolygon& p)
{
	plist.push_back(p);
}
void SGLPolygonList::addPolygon(const SGLPolygon* pbuffer, int count)
{
	plist.reserve(count * 2);
	if(plist.empty())
		plist.push_back(SGLPolygon());
	memcpy(&plist[0], pbuffer, sizeof(SGLPolygon)*count);
}
void SGLPolygonList::addPolygon(const int* indexBuffer, int count)
{
	SGLPolygon poly;
	poly.indexList.addIndex(indexBuffer, count);
	poly.numVertex = count;
}
SGLPolygon& SGLPolygonList::getLastPolygon(void)
{
	return plist.back();
}
SGLPolygon& SGLPolygonList::operator[](unsigned int index)
{
	return plist[index];
}
const SGLPolygon& SGLPolygonList::operator[](unsigned int index) const
{
	return plist[index];
}

int SGLPolygonList::size(void) const
{
	return plist.size();
}


//------------------------------
SGLVertexPolygonList::SGLVertexPolygonList(void)
{
	clear();
	vplist.reserve(1024);
}

SGLVertexPolygonList::~SGLVertexPolygonList(void)
{
	clear();
}
void SGLVertexPolygonList::clear(void)
{
	vplist.clear();
}
void SGLVertexPolygonList::addPolygon(const SGLVertexPolygon& p)
{
	vplist.push_back(p);
}
void SGLVertexPolygonList::addPolygon(const SGLVertexPolygon* pbuffer, int count)
{
	vplist.reserve(count * 2);
	if(vplist.empty())
		vplist.push_back(SGLVertexPolygon());
	memcpy(&vplist[0], pbuffer, sizeof(SGLVertexPolygon)*count);
}
SGLVertexPolygon& SGLVertexPolygonList::getLastPolygon(void)
{
	return vplist.back();
}
SGLVertexPolygon& SGLVertexPolygonList::operator[](unsigned int index)
{
	return vplist[index];
}
const SGLVertexPolygon& SGLVertexPolygonList::operator[](unsigned int index) const
{
	return vplist[index];
}

int SGLVertexPolygonList::size(void) const
{
	return vplist.size();
}