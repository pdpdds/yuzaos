#ifndef SGLMATRIXSTACK_H
#define SGLMATRIXSTACK_H

#include <stack>
#include "SGLMatrix44.h"
class SGLMatrixStack
{
private:
	std::stack<SGLMatrix44> mstack;
public:
	SGLMatrixStack(void);
	~SGLMatrixStack(void);
	void init(void);
	void push(void);
	void mul(const SGLMatrix44& m);
	void pop(SGLMatrix44* m = 0);
	void setIdentityTopMatrix(void);
	const SGLMatrix44& getTop(void) const;
	SGLMatrix44& getTop();
};

#endif