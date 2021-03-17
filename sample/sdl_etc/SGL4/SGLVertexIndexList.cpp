#include "SGLVertexIndexList.h"

SGLVertexIndexList::SGLVertexIndexList(void)
{
	clear();
	vilist.reserve(1024);
}

SGLVertexIndexList::~SGLVertexIndexList(void)
{
	clear();
}

void SGLVertexIndexList::clear(void)
{
	vilist.clear();
}
void SGLVertexIndexList::addIndex(int index)
{
	vilist.push_back(index);
}
void SGLVertexIndexList::addIndex(const int* indexBuffer, int count)
{
	vilist.reserve(count * 2); //두 배의 개수로 메모리 예약
	for(int i=0; i<count; i++)
	{
		vilist.push_back(indexBuffer[i]);
	}
}
int& SGLVertexIndexList::getLastIndex(void)
{
	return vilist.back();
}
int& SGLVertexIndexList::operator[](unsigned int index)
{
	return vilist[index];
}
const int& SGLVertexIndexList::operator[](unsigned int index) const
{
	return vilist[index];
}
int SGLVertexIndexList::size(void) const
{
	return vilist.size();
}