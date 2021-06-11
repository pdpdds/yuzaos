#pragma once
#include "minwindef.h"

#pragma pack( push, 1 )
// �����͸� �����ϴ� �ڷᱸ��
// �ݵ�� �������� ���� �պκп� ��ġ�ؾ� ��
typedef struct kListLinkStruct
{
    // ���� �������� ��巹���� �����͸� �����ϱ� ���� ID
    void* pvNext;
    QWORD qwID;
} LISTLINK;

// ����Ʈ�� �����ϴ� �ڷᱸ��
typedef struct kListManagerStruct
{
    // ����Ʈ �������� ��
    int iItemCount;

    // ����Ʈ�� ù ��°�� ������ �������� ��巹��
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
