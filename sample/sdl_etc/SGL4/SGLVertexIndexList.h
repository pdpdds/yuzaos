#ifndef SGLVERTEXINDEXLIST_H
#define SGLVERTEXINDEXLIST_H

#include <vector>
class SGLVertexIndexList
{
public:
	std::vector<int> vilist;
public:
	SGLVertexIndexList(void);
	~SGLVertexIndexList(void);
	void clear(void);
	void addIndex(int index);
	void addIndex(const int* indexBuffer, int count);
	int& getLastIndex(void);
	int& operator[](unsigned int index);
	const int& operator[](unsigned int index) const;
	int size(void) const;
};

#define SGLTextureIdList SGLVertexIndexList 

#endif