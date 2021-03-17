#ifndef SGLTEXTURELIST_H
#define SGLTEXTURELIST_H

#include <vector>
class SGLTexture;
class SGLTextureList
{
public:
	std::vector<SGLTexture*> texlist;
public:
	SGLTextureList(void);
	~SGLTextureList(void);
	void clear(void);
	int addTexture(const char* filename);
	SGLTexture* operator[](unsigned int index);
	const SGLTexture* operator[](unsigned int index) const;
	int size(void) const;
};

#endif