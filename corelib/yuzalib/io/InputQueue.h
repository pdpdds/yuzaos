#pragma once

#pragma pack( push, 1 )

// ť�� ���� ����ü
typedef struct kQueueManagerStruct
{
    // ť�� �����ϴ� ������ �ϳ��� ũ��� �ִ� ����
    int iDataSize;
    int iMaxDataCount;

    // ť ������ �����Ϳ� ����/���� �ε���
    void* pvQueueArray;
    int iPutIndex;
    int iGetIndex;
    
    // ť�� ����� ������ ����� ���������� ����
    bool bLastOperationPut;
} QUEUE;

#pragma pack( pop )

void kInitializeQueue( QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, 
		int iDataSize );
bool kIsQueueFull( const QUEUE* pstQueue );
bool kIsQueueEmpty( const QUEUE* pstQueue );
bool kPutQueue( QUEUE* pstQueue, const void* pvData );
bool kGetQueue( QUEUE* pstQueue, void* pvData );
