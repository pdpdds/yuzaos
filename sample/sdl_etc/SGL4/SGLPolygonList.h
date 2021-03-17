#ifndef SGLPOLYGONLIST_H
#define SGLPOLYGONLIST_H

#include "SGLPolygon.h"
#include "SGLConst.h"
#include <vector>
class SGLPolygonList
{
public:
	std::vector<SGLPolygon> plist;
public:
	SGLPolygonList(void);
	~SGLPolygonList(void);
	void clear(void);
	void addPolygon(const SGLPolygon& p);
	void addPolygon(const SGLPolygon* pbuffer, int count);
	void addPolygon(const int* indexBuffer, int count);
	SGLPolygon& getLastPolygon(void);
	SGLPolygon& operator[](unsigned int index);
	const SGLPolygon& operator[](unsigned int index) const;
	int size(void) const;
};

class SGLVertexPolygonList
{
public:
	std::vector<SGLVertexPolygon> vplist;
public:
	SGLVertexPolygonList(void);
	~SGLVertexPolygonList(void);
	void clear(void);
	void addPolygon(const SGLVertexPolygon& p);
	void addPolygon(const SGLVertexPolygon* pbuffer, int count);
	SGLVertexPolygon& getLastPolygon(void);
	SGLVertexPolygon& operator[](unsigned int index);
	const SGLVertexPolygon& operator[](unsigned int index) const;
	int size(void) const;
};


#endif
