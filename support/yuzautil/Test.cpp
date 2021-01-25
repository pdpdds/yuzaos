/*#include <tchar.h>
#include "bitarray.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// initialize CBitArray object
	CBitArray a;
	// set the bit 4578 in the bit array buffer (at byte 4578/8, at bit 4578%8)
	a.SetAt(4578);
	// set the bit 323 in the bit array buffer (at byte 323/8, at bit 323%8)
	a.SetAt(323);
	// get the count of '1's
	int nCount = a.GetCount(); // return 2
	// xor bit number 323
	a.XOrAt(323);
	// get the count of '1's
	nCount = a.GetCount(); // return 1
	// initialize CBitArray object
	CBitArray b;
	// attach buffer which is allocated with some bytes
	//char* buffer[] = {1,1,1,1};
	//b.Attach((BYTE*)buffer, 4); 
	// AND b with a
	
	// ...
	// and so on

	BYTE* pBuffer = new BYTE[500000];
	memset(pBuffer, 0, 500000);
	b.Attach(pBuffer, 500000);
	b.SetAt(100000);
	b.SetAt(999999);

////////////////////////////////////////////////////
//5행 5열 타일에서 5행 6열의 타일이 보이는가?
//5행 5열은 25번째 인덱스 5행 6열은 26번째 인덱스
////////////////////////////////////////////////////
//25* 100 + 250???
	nCount = b.GetCount();

	return 0;
}*/

/*#include "SString.h"

#define MAX_NICKNAME_LEN 8
DECLARE_STRING_TYPE(SSNickName, MAX_NICKNAME_LEN);

int _tmain(int argc, _TCHAR* argv[])
{
	SSNickName szName;
	szName = _T("Juhang");

	return 0;
}*/

#include <tchar.h>
#include <winapi.h>
#include "LockFreeQueue.h"
#include "LockFreeStack.h"
#include "GPGLockFree.h"
#include "MSLockFreeQ.h"


int _tmain(int argc, _TCHAR* argv[])
{
	CLockFreeQueue<int> cLockFreeQueue;
	cLockFreeQueue.Enqueue(100);

	CLockFreeStack<int> cLockFreeStack;

	cLockFreeStack.Push(5);
	cLockFreeStack.Push(6);
	cLockFreeStack.Push(7);
	cLockFreeStack.Push(8);

	int a = 0;
	while (true == cLockFreeStack.Pop(a))
	{
		printf("%d\n", a);
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	GPG::node<int> GPGNode;
	GPG::node<int> GPGNode1;
	GPGNode1.value = 5;
	GPG::node<int> GPGNode2;
	GPGNode2.value = 6;
	GPG::node<int> GPGNode3;
	GPGNode3.value = 7;

	GPG::LockFreeQueue<int> GPGQueue(&GPGNode);

	GPGQueue.Add(&GPGNode1);
	GPGQueue.Add(&GPGNode2);
	GPGQueue.Add(&GPGNode3);

	GPG::node<int>* pNode = GPGQueue.Remove();

	while (pNode != NULL)
	{
		printf("%d\n", pNode->value);

		pNode = GPGQueue.Remove();
	}

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////


	MS::MSQueue< int > Q;

	for (int i = 0; i < 10; i++)
	{
		Q.enqueue(i);
	}

	cout << "Contents of queue." << endl;

	for (int j = 0; j < 11; j++)
	{
		int n;
		if (Q.dequeue(n))
		{
			cout << n << endl;
		}
	}


	return 0;
}
