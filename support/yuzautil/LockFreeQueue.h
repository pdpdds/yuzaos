#pragma once
// Copyright Idaho O Edokpayi 2008
// Code is governed by Code Project Open License 

//#include <exception>
#include <minwindef.h>
#include <algorithm>

using namespace std;

/////////////////////////
// Array based lock free
// queue 
/////////////////////////
template<class T>
class ArrayQ
{
private:
	T* pData;
	volatile LONG nWrite;
	volatile LONG nRead;
	volatile LONG nSize;
	// size of array at creation
	enum SizeEnum{ InitialSize=240 };
	// Lock pData to copy
	void Resize()
	{
		// Declare temporary size variable
		LONG nNewSize = 0;		
		CRITICAL_SECTION cs;

		// double the size of our queue
		InterlockedExchangeAdd(&nNewSize,2 * nSize);

		// allocate the new array
		T* pNewData = new T[nNewSize];
		const ULONG uiTSize = sizeof(T);

		// Initialize the critical section to protect the copy
		InitializeCriticalSection(&cs);

		// Enter the critical section
		EnterCriticalSection(&cs);

		// copy the old data
		memcpy_s((void*)pNewData,nNewSize*uiTSize,(void*)pData,nSize*uiTSize);		

		// dump the old array
		delete[] pData;

		// save the new array
		pData = pNewData;

		// save the new size
		nSize = nNewSize;

		// Leave the critical section
		LeaveCriticalSection(&cs);

		// Delete the critical section
		DeleteCriticalSection(&cs);
	}
public:
	ArrayQ()	: nWrite(0), nRead(0), pData(new T[InitialSize]), nSize(InitialSize)
	{

	}

	~ArrayQ()
	{
		delete[] pData;
	}


	void enqueue( const T& t ) 
	{
		// temporary write index and size
		volatile LONG nTempWrite, nTempSize;

		// atomic copy of the originals to temporary storage
		InterlockedExchange(&nTempWrite,nWrite);
		InterlockedExchange(&nTempSize,nSize);

		// increment before bad things happen
		InterlockedIncrement(&nWrite);

		// check to make sure we haven't exceeded our storage 
		if(nTempWrite == nTempSize)
		{
			// we should resize the array even if it means using a lock
			Resize();			
		}

		pData[nTempWrite] = t;		
	}

	// returns false if queue is empty
	bool dequeue( T& t ) 
	{
		// temporary write index and size
		volatile LONG nTempWrite, nTempRead;

		// atomic copy of the originals to temporary storage
		InterlockedExchange(&nTempWrite,nWrite);
		InterlockedExchange(&nTempRead,nRead);

		// increment before bad things happen
		InterlockedIncrement(&nRead);

		// check to see if queue is empty
		if(nTempRead == nTempWrite)
		{
			// reset both indices
			InterlockedCompareExchange(&nRead,0,nTempRead+1);
			InterlockedCompareExchange(&nWrite,0,nTempWrite);
			return false;
		}

		t = pData[nTempRead];
		return true;
	}

};



template< class T >
class CLockFreeQueue
{
private:

	// pointer structure
	struct node_t;

	struct pointer_t 
	{
		node_t* ptr;
		LONG count;
		// default to a null pointer with a count of zero
		pointer_t(): ptr(NULL),count(0){}
		pointer_t(node_t* node, const LONG c ) : ptr(node),count(c){}
		pointer_t(const pointer_t& p)
		{
			InterlockedExchange(&count,p.count);
			InterlockedExchangePointer((PVOID volatile*)&ptr,p.ptr);
		}

		pointer_t(const pointer_t* p): ptr(NULL),count(0)
		{
			if(NULL == p)
				return;

			InterlockedExchange(&count,const_cast< LONG >(p->count));
			InterlockedExchangePointer(ptr,const_cast< node_t* >(p->ptr));			
		}

	};

	// node structure
	struct node_t 
	{
		T value;
		pointer_t next;
		// default constructor
		node_t(){}
	};

	pointer_t Head;
	pointer_t Tail;
	bool CAS(pointer_t& dest,pointer_t& compare, pointer_t& value)
	{
		if(compare.ptr==InterlockedCompareExchangePointer((PVOID volatile*)&dest.ptr,value.ptr,compare.ptr))
		{
			InterlockedExchange(&dest.count,value.count);
			return true;
		}

		return false;
	}
public:	
	// default constructor
	CLockFreeQueue()
	{
		node_t* pNode = new node_t();
		Head.ptr = Tail.ptr = pNode;
	}
	~CLockFreeQueue()
	{
		// remove the dummy head
		delete Head.ptr;
	}

	// insert items of class T in the back of the queue
	// items of class T must implement a default and copy constructor
	// Enqueue method
	void Enqueue(const T& t)
	{
		// Allocate a new node from the free list
		node_t* pNode = new node_t(); 

		// Copy enqueued value into node
		pNode->value = t;

		// Keep trying until Enqueue is done
		bool bEnqueueNotDone = true;

		while(bEnqueueNotDone)
		{
			// Read Tail.ptr and Tail.count together
			pointer_t tail(Tail);

			bool nNullTail = (NULL==tail.ptr); 
			// Read next ptr and count fields together
			pointer_t next( // ptr 
							(nNullTail)? NULL : tail.ptr->next.ptr,
							// count
							(nNullTail)? 0 : tail.ptr->next.count
							) ;


			// Are tail and next consistent?
			if(tail.count == Tail.count && tail.ptr == Tail.ptr)
			{
				if(NULL == next.ptr) // Was Tail pointing to the last node?
				{
					// Try to link node at the end of the linked list										
					if(CAS( tail.ptr->next, next, pointer_t(pNode,next.count+1) ) )
					{
						bEnqueueNotDone = false;
					} // endif

				} // endif

				else // Tail was not pointing to the last node
				{
					// Try to swing Tail to the next node
					CAS(Tail, tail, pointer_t(next.ptr,tail.count+1) );
				}

			} // endif

		} // endloop
	}

	// remove items of class T from the front of the queue
	// items of class T must implement a default and copy constructor
	// Dequeue method
	bool Dequeue(T& t)
	{
		pointer_t head;
		// Keep trying until Dequeue is done
		bool bDequeNotDone = true;
		while(bDequeNotDone)
		{
			// Read Head
			head = Head;
			// Read Tail
			pointer_t tail(Tail);

			if(head.ptr == NULL)
			{
				// queue is empty
				return false;
			}

			// Read Head.ptr->next
			pointer_t next(head.ptr->next);

			// Are head, tail, and next consistent
			if(head.count == Head.count && head.ptr == Head.ptr)
			{
				if(head.ptr == tail.ptr) // is tail falling behind?
				{
					// Is the Queue empty
					if(NULL == next.ptr)
					{
						// queue is empty cannot deque
						return false;
					}
					CAS(Tail,tail, pointer_t(next.ptr,tail.count+1)); // Tail is falling behind. Try to advance it
				} // endif

				else // no need to deal with tail
				{
					// read value before CAS otherwise another deque might try to free the next node
					t = next.ptr->value;

					// try to swing Head to the next node
					if(CAS(Head,head, pointer_t(next.ptr,head.count+1) ) )
					{
						bDequeNotDone = false;
					}
				}

			} // endif

		} // endloop
		
		// It is now safe to free the old dummy node
		delete head.ptr;

		// queue was not empty, deque succeeded
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////
/// \file LockFreeQueue.h
/// \author excel96
/// \date 2006.9.29
////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////
/// \class cLockFreeQueue
/// \brief 락을 따로 사용하지 않는 큐. 98 이상에서는 사용 가능하다.
////////////////////////////////////////////////////////////////////////////////

/*template 
<
    typename T,
    template <typename> class AL = std::allocator
>
class cLockFreeQueue
{
private:
#pragma pack(push)
#pragma pack(MEMORY_ALLOCATION_ALIGNMENT)
    struct NODE
    {
        NODE* Next;
        T     Data;
    };
#pragma pack(pop)

    NODE*    m_Head;      ///< 큐 헤드.
    NODE*    m_Tail;      ///< 큐 테일.
    AL<NODE> m_Allocator; ///< 메모리 할당자.


public:
    /// \brief 생성자.
    cLockFreeQueue()
    {
        m_Head = m_Allocator.allocate(1);
        m_Allocator.construct(m_Head, NODE());

        m_Tail = m_Head;
        m_Head->Next = NULL;
        m_Tail->Next = NULL;
    }

    /// \brief 소멸자.
    ~cLockFreeQueue()
    {
        Clear();
        m_Allocator.destroy(m_Head);
        m_Allocator.deallocate(m_Head, 0);
    }


public:
    /// \brief 데이터를 추가한다.
    /// \param data 추가할 데이터.
    void Push(const T& data)
    {
        NODE* oldTail = NULL;
        NODE* oldNext = NULL;

        // 새로운 노드를 할당한다.
        NODE* node = m_Allocator.allocate(1);
        m_Allocator.construct(node, NODE());
        node->Next = NULL;
        node->Data = data;

        // 테일 쪽의 Next 링크가 새로운 노드를 가르킬 때까지 루프를 돈다.
        bool addedNewNode = false;
        while (!addedNewNode) 
        {
            // 포인터들의 카피를 만든다.
            // 다른 스레드에서 테일 포인터를 변경할 수 있기 때문이다.
            oldTail = m_Tail;         
            oldNext = oldTail->Next; 

            // 테일 포인터가 변경되지 않았다면...
            if (m_Tail == oldTail) 
            {
                // 테일의 Next 포인터가 NULL이라면...
                if (oldNext == NULL) 
                {
                    // 테일의 Next 포인터 변경을 시도한다.
                    addedNewNode = CAS(&(m_Tail->Next), NULL, node);
                }
                // 테일의 Next 포인터가 NULL이 아니라는 말은, 
                // 다른 스레드에서 노드를 추가하고 있는 중이라는 말이다. 
                // 그러므로 테일 포인터 변경을 시도한다.
                else 
                {
                    CAS(&m_Tail, oldTail, oldNext);
                }
            }
        }

        // 테일 포인터가 새로운 노드를 가르키도록 변경한다. 실패할 수도 있는데,
        // 다른 스레드에서 처리해줄 것이므로 걱정할 필요 없다.
        CAS(&m_Tail, oldTail, node);
    }

    /// \brief 데이터를 뽑아낸다.
    /// \param data 뽑아낸 데이터를 쓸 변수.
    /// \return bool 데이터를 뽑아낸 경우에는 true를 반환하고, 
    /// 데이터가 없는 경우에는 false를 반환한다.
    bool Pop(T& data)
    {
        NODE* oldHead = NULL;
        NODE* oldTail = NULL;
        NODE* oldHeadNext = NULL;

        // 헤드 포인터가 다음 노드를 가르킬 때까지 루프를 돈다.
        bool advancedHead = false;
        while (!advancedHead) 
        {
            // 포인터들의 카피를 만든다.
            // 다른 스레드에서 테일 포인터를 변경할 수 있기 때문이다.
            oldHead = m_Head;
            oldTail = m_Tail;
            oldHeadNext = oldHead->Next;

            // 헤드 포인터가 변경되지 않았다면...
            if (oldHead == m_Head) 
            {
                // 헤드 포인터가 테일 포인터와 같다면...
                if (oldHead == oldTail) 
                {
                    // 헤드의 Next 포인터가 NULL이라면...
                    if (oldHeadNext == NULL) 
                    {
                        // 뽑아낼 아이템이 존재하지 않는다.
                        return false;
                    }

                    // 헤드의 Next 포인터가 NULL이 아니고, 
                    // 헤드가 테일과 같지 않다는 말은,
                    // 다른 스레드에서 테일에다 노드를 추가하고 있다는 말이다. 
                    // 테일 포인터를 업데이트한다.
                    CAS(&m_Tail, oldTail, oldHeadNext);
                }
                // 헤드 포인터가 테일 포인터와 다르다면...
                else 
                {
                    // 뽑아낼 아이템을 복사하고, 헤드 포인터를 뒤로 옮긴다.
                    data = oldHeadNext->Data;
                    advancedHead = CAS(&m_Head, oldHead, oldHeadNext);
                }
            }
        }

        m_Allocator.destroy(oldHead);
        m_Allocator.deallocate(oldHead, 0);

        return true;
    }

    /// \brief 모든 데이터를 삭제한다.
    void Clear()
    {
        T data;
        while (Pop(data))
            ;
    }


private:
    /// Compare and Swap
    bool CAS(NODE** location, NODE* comperand, NODE* newValue)
    {
        return comperand == InterlockedCompareExchangePointer(
            reinterpret_cast<volatile PVOID*>(location), newValue, comperand
            );
    }

    /// \brief 복사 생성 금지.
    cLockFreeQueue(const cLockFreeQueue&) {}

    /// \brief 대입 금지.
    cLockFreeQueue& operator = (const cLockFreeQueue&) { return *this; }
};*/