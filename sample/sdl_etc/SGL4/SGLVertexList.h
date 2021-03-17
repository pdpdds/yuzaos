#ifndef SGLVERTEXLIST_H
#define SGLVERTEXLIST_H
#include "SGLVertex.h"
#include "SGLConst.h"
#include <vector>
class SGLVertexList
{
private:
	std::vector<SGLVertex> vlist;
public:
	SGLVertexList(void);
	~SGLVertexList(void);
	void clear(void);
	void addVertex(const SGLVertex& v);
	void addVertex(const SGLVertex* vbuffer, int count);
	SGLVertex& getLastVertex(void);
	SGLVertex& operator[](unsigned int index);
	const SGLVertex& operator[](unsigned int index) const;
	int size(void) const;
};

#endif
