#ifndef SGLOBJECTLIST_H
#define SGLOBJECTLIST_H

#include "SGLObject.h"
#include <vector>
//템플릿으로 구현할걸 -_-;;
class SGLObjectList
{
private:
	std::vector<SGLObject> olist;
public:
	SGLObjectList(void);
	~SGLObjectList(void);
	void clear(void);
	void addObject(const SGLObject& o);
	void addObject(const SGLObject* obuffer, int count);
	SGLObject& getLastObject(void);
	SGLObject& operator[](unsigned int index);
	const SGLObject& operator[](unsigned int index) const;
	int size(void) const;
};

#endif