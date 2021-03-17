#include "SGLMatrixStack.h"
using namespace std;

SGLMatrixStack::SGLMatrixStack(void)
{
	init();
}

SGLMatrixStack::~SGLMatrixStack(void)
{
}

void SGLMatrixStack::init(void)
{
	while( !mstack.empty() )
		mstack.pop();
	SGLMatrix44 m;
	m.identity();
	mstack.push(m);
}
void SGLMatrixStack::setIdentityTopMatrix(void)
{
	if(!mstack.empty())
	{
		SGLMatrix44& m = mstack.top();
		m.identity();
	}
}
void SGLMatrixStack::push(void)
{//Top 의 행렬을 다시 넣는다.
	mstack.push(mstack.top());
}
void SGLMatrixStack::mul(const SGLMatrix44& m)
{//Top 의 행렬과 곱한다.
	mstack.top() *= m;
}
void SGLMatrixStack::pop(SGLMatrix44* m)
{
	if(mstack.empty())
		return ;
	if(m != 0)
		*m = mstack.top();
	mstack.pop();
}
const SGLMatrix44& SGLMatrixStack::getTop(void) const
{
	return mstack.top();
}

SGLMatrix44& SGLMatrixStack::getTop(void)
{
	return mstack.top();
}