#include "InputQueue.h"
#include "minwindef.h"
#include "memory.h"
/**
 *  ť�� �ʱ�ȭ
 */
void kInitializeQueue( QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, 
		int iDataSize )
{
    // ť�� �ִ� ������ ũ��, �׸��� ���� ��巹���� ����
	pstQueue->iMaxDataCount = iMaxDataCount;
	pstQueue->iDataSize = iDataSize;
	pstQueue->pvQueueArray = pvQueueBuffer;

    // ť�� ���� ��ġ�� ���� ��ġ�� �ʱ�ȭ�ϰ� ���������� ����� ����� ���ŷ�
    // �����Ͽ� ť�� �� ���·� ����
	pstQueue->iPutIndex = 0;
	pstQueue->iGetIndex = 0;
	pstQueue->bLastOperationPut = FALSE;
}

/**
 *  ť�� ���� á���� ���θ� ��ȯ
 */
bool kIsQueueFull( const QUEUE* pstQueue )
{
    // ť�� ���� �ε����� ���� �ε����� ���� ���������� ����� ����� �����̸�
    // ť�� ���� á���Ƿ� ������ �� ����
    if( ( pstQueue->iGetIndex == pstQueue->iPutIndex ) &&
        ( pstQueue->bLastOperationPut == TRUE ) )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  ť�� ������� ���θ� ��ȯ
 */
bool kIsQueueEmpty( const QUEUE* pstQueue )
{
    // ť�� ���� �ε����� ���� �ε����� ���� ���������� ����� ����� �����̸�
    // ť�� ������Ƿ� ������ �� ����
    if( ( pstQueue->iGetIndex == pstQueue->iPutIndex ) &&
        ( pstQueue->bLastOperationPut == FALSE ) )
    {
        return TRUE;
    }
    return FALSE;
}   

/**
 * 	ť�� �����͸� ����
 */
bool kPutQueue( QUEUE* pstQueue, const void* pvData )
{
    // ť�� ���� á���� ������ �� ����
	if( kIsQueueFull( pstQueue ) == TRUE )
	{
	    return FALSE;
	}
	
	// ���� �ε����� ����Ű�� ��ġ���� �������� ũ�⸸ŭ�� ����
	memcpy( ( char* ) pstQueue->pvQueueArray + ( pstQueue->iDataSize * 
			pstQueue->iPutIndex ), pvData, pstQueue->iDataSize );
	
    // ���� �ε����� �����ϰ� ���� ������ ���������� ���
	pstQueue->iPutIndex = ( pstQueue->iPutIndex + 1 ) % pstQueue->iMaxDataCount;
	pstQueue->bLastOperationPut = TRUE;
	return TRUE;
}

/**
 * 	ť���� �����͸� ����
 */
bool kGetQueue( QUEUE* pstQueue, void* pvData )
{
    // ť�� ������� ������ �� ����
    if( kIsQueueEmpty( pstQueue ) == TRUE )
    {
        return FALSE;
    }
	
	// ���� �ε����� ����Ű�� ��ġ���� �������� ũ�⸸ŭ�� ����
	memcpy( pvData, ( char* ) pstQueue->pvQueueArray + ( pstQueue->iDataSize * 
			 pstQueue->iGetIndex ), pstQueue->iDataSize );
	
    // ���� �ε����� �����ϰ� ���� ������ ���������� ���
	pstQueue->iGetIndex = ( pstQueue->iGetIndex + 1 ) % pstQueue->iMaxDataCount;
    pstQueue->bLastOperationPut = FALSE;
	return TRUE;
}