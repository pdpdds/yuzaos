#pragma once
#include "minwindef.h"

#pragma pack( push, 1 )
// 데이터를 연결하는 자료구조
// 반드시 데이터의 가장 앞부분에 위치해야 함
typedef struct kListLinkStruct
{
    // 다음 데이터의 어드레스와 데이터를 구분하기 위한 ID
    void* pvNext;
    QWORD qwID;
} LISTLINK;

// 리스트를 관리하는 자료구조
typedef struct kListManagerStruct
{
    // 리스트 데이터의 수
    int iItemCount;

    // 리스트의 첫 번째와 마지막 데이터의 어드레스
    void* pvHeader;
    void* pvTail;
} LIST;

#pragma pack( pop )


void kInitializeList( LIST* pstList );
int kGetListCount( const LIST* pstList );
void kAddListToTail( LIST* pstList, void* pvItem );
void kAddListToHeader( LIST* pstList, void* pvItem );
void* kRemoveList( LIST* pstList, QWORD qwID );
void* kRemoveListFromHeader( LIST* pstList );
void* kRemoveListFromTail( LIST* pstList );
void* kFindList( const LIST* pstList, QWORD qwID );
void* kGetHeaderFromList( const LIST* pstList );
void* kGetTailFromList( const LIST* pstList );
void* kGetNextFromList( const LIST* pstList, void* pstCurrent );
