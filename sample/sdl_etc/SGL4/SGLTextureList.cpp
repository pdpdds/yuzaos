#include "SGLTextureList.h"
#include "SGLTexture.h"
SGLTextureList::SGLTextureList(void)
{
	clear();
}

SGLTextureList::~SGLTextureList(void)
{
	clear();
}

void SGLTextureList::clear(void)
{
	for(int i=0; i<texlist.size(); i++)
	{
		if(texlist[i])
			delete texlist[i];
	}
	texlist.clear();
	texlist.reserve(1024);
}
int SGLTextureList::addTexture(const char* filename)
{
	texlist.push_back(new SGLTexture(filename));
	return size()-1;
}
SGLTexture* SGLTextureList::operator[](unsigned int index)
{
	return texlist[index];
}
const SGLTexture* SGLTextureList::operator[](unsigned int index) const
{
	return texlist[index];
}
int SGLTextureList::size(void) const
{
	return texlist.size();
}