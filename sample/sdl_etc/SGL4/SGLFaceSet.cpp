#include "SGLFaceSet.h"

SGLFaceSet::SGLFaceSet(void)
{
	clear();
}

SGLFaceSet::~SGLFaceSet(void)
{
	clear();
}
void SGLFaceSet::clear(void)
{
	faceset.clear();
	faceset.reserve(1024);
}
void SGLFaceSet::addFace(const SGLFace& f)
{
	faceset.push_back(f);
}
SGLFace& SGLFaceSet::operator[](unsigned int index)
{
	return faceset[index];
}
const SGLFace& SGLFaceSet::operator[](unsigned int index) const
{
	return faceset[index];
}
int SGLFaceSet::size(void) const
{
	return faceset.size();
}