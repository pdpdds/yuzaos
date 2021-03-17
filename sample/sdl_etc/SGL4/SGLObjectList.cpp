#include "SGLObjectList.h"

SGLObjectList::SGLObjectList(void)
{
	clear();
	olist.reserve(1024);
}

SGLObjectList::~SGLObjectList(void)
{
	clear();
}

void SGLObjectList::clear(void)
{
	olist.clear();
}
void SGLObjectList::addObject(const SGLObject& v)
{
	olist.push_back(v);
}
void SGLObjectList::addObject(const SGLObject* obuffer, int count)
{
	olist.reserve(count * 2); //두 배의 개수로 메모리 예약
	if(olist.empty()) //아래 메모리복사가 실패하지 않도록 임의의 버텍스를 한 개 넣는다
		olist.push_back(SGLObject());
	memcpy(&olist[0], obuffer, sizeof(SGLObject)*count);
}
SGLObject& SGLObjectList::getLastObject(void)
{
	return olist.back();
}
SGLObject& SGLObjectList::operator[](unsigned int index)
{
	return olist[index];
}
const SGLObject& SGLObjectList::operator[](unsigned int index) const
{
	return olist[index];
}

int SGLObjectList::size(void) const
{
	return olist.size();
}