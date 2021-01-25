#pragma once
#include <stl_alloc.h>
#include <list>
#include <winnt.h>

template <typename T, template <typename> class AL = std::allocator>


class CLockFreeStack
{
private:
#pragma pack(push)
#pragma pack(MEMORY_ALLOCATION_ALIGNMENT)
    /// 유저 데이터를 저장하기 위한 구조체.
    struct NODE
    {
        SLIST_ENTRY List;
        T           Data;
    };
#pragma pack(pop)

    SLIST_HEADER m_ListHead;  ///< 리스트 헤드.
    AL<NODE>     m_Allocator; ///< 메모리 할당자.


public:
    /// \brief 생성자.
    CLockFreeStack()
    {
        InitializeSListHead(&m_ListHead);
    }

    /// \brief 소멸자.
    ~CLockFreeStack()
    {
        Clear();
    }


public:
    /// \brief 데이터를 추가한다.
    /// \param data 추가할 데이터.
    void Push(const T& data)
    {
        NODE* newNode = m_Allocator.allocate(1);
        m_Allocator.construct(newNode, NODE());
        newNode->Data = data;
        InterlockedPushEntrySList(&m_ListHead, &newNode->List);
    }

    /// \brief 데이터를 뽑아낸다.
    /// \param data 뽑아낸 데이터를 쓸 변수.
    /// \return bool 데이터를 뽑아낸 경우에는 true를 반환하고, 
    /// 데이터가 없는 경우에는 false를 반환한다.
    bool Pop(T& data)
    {
        PSLIST_ENTRY entry = InterlockedPopEntrySList(&m_ListHead);
        if (entry)
        {
            NODE* node = reinterpret_cast<NODE*>(entry);
            data = node->Data;
            m_Allocator.destroy(node);
            m_Allocator.deallocate(node, 0);
            return true;
        }

        return false;
    }

    /// \brief 모든 데이터를 삭제한다.
    void Clear()
    {
        T data;
        while (Pop(data))
            ;
    }


private:
    /// \brief 복사 생성 금지.
    CLockFreeStack(const CLockFreeStack&) {}

    /// \brief 대입 금지.
    CLockFreeStack& operator = (const CLockFreeStack&) { return *this; }
};



