#include "SGLVertexList.h"

SGLVertexList::SGLVertexList(void)
{
	clear();
	vlist.reserve(1024);
}

SGLVertexList::~SGLVertexList(void)
{
	clear();
}

void SGLVertexList::clear(void)
{
	vlist.clear();
}
void SGLVertexList::addVertex(const SGLVertex& v)
{
	vlist.push_back(v);
}
void SGLVertexList::addVertex(const SGLVertex* vbuffer, int count)
{
	vlist.reserve(count * 2); //두 배의 개수로 메모리 예약
	for(int i=0; i<count; i++)
	{
		vlist.push_back(vbuffer[i]);
	}
}
SGLVertex& SGLVertexList::getLastVertex(void)
{
	return vlist.back();
}
SGLVertex& SGLVertexList::operator[](unsigned int index)
{
	return vlist[index];
}
const SGLVertex& SGLVertexList::operator[](unsigned int index) const
{
	return vlist[index];
}
int SGLVertexList::size(void) const
{
	return vlist.size();
}