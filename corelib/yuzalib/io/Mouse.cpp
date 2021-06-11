#include "Mouse.h"
#include "Keyboard.h"
#include "InputQueue.h"
#include "systemcall_impl.h"
#include <_spinlock.h>

Spinlock_t g_mouse_spinlock = SPINLOCK_INIT;
// ���콺 ���¸� �����ϴ� ���콺 �Ŵ���
static MOUSEMANAGER gs_stMouseManager = { 0, };

// ���콺�� �����ϴ� ť�� ���� ����
QUEUE gs_stMouseQueue;
static MOUSEDATA gs_vstMouseQueueBuffer[ MOUSE_MAXQUEUECOUNT ];

/**
 *  ���콺 �ʱ�ȭ
 */
bool kInitializeMouse( void )
{
    // ť �ʱ�ȭ
    kInitializeQueue( &gs_stMouseQueue, gs_vstMouseQueueBuffer, MOUSE_MAXQUEUECOUNT, 
            sizeof( MOUSEDATA ) );
   
    return FALSE;
}

/**
 *  ���콺 �����͸� ��Ƽ� ť�� ����
 */
bool kAccumulateMouseDataAndPutQueue( BYTE bMouseData )
{
	bool bResult;
    
    // ���ŵ� ����Ʈ ���� ���� ���콺 �����͸� ����
    switch( gs_stMouseManager.iByteCount )
    {
        // ����Ʈ 1�� ������ ����
    case 0:
        gs_stMouseManager.stCurrentData.bButtonStatusAndFlag = bMouseData;
        gs_stMouseManager.iByteCount++;
        break;
        
        // ����Ʈ 2�� ������ ����
    case 1:
        gs_stMouseManager.stCurrentData.bXMovement = bMouseData;
        gs_stMouseManager.iByteCount++;
        break;
        
        // ����Ʈ 3�� ������ ����
    case 2:
        gs_stMouseManager.stCurrentData.bYMovement = bMouseData;
        gs_stMouseManager.iByteCount++;
        break;
        
        // �� ���� ���� ���ŵ� ����Ʈ �� �ʱ�ȭ
    default:
        gs_stMouseManager.iByteCount = 0;
        break;
    }
    
    // 3����Ʈ�� ��� ���ŵǾ����� ���콺 ť�� �����ϰ� ���ŵ� Ƚ���� �ʱ�ȭ
    if( gs_stMouseManager.iByteCount >= 3 )
    {
       
		SpinlockAcquire(&g_mouse_spinlock);
        bResult = kPutQueue( &gs_stMouseQueue, &gs_stMouseManager.stCurrentData );
		SpinlockRelease(&g_mouse_spinlock);

        // ���ŵ� ����Ʈ �� �ʱ�ȭ
        gs_stMouseManager.iByteCount = 0;
    }
    return bResult;
}

/**
 *  ���콺 ť���� ���콺 �����͸� ����
 */
bool kGetMouseDataFromMouseQueue( BYTE* pbButtonStatus, int* piRelativeX, int* piRelativeY, bool& bAbsoluteCoordinate)
{
    MOUSEDATA stData;
	bool bResult;

	SpinlockAcquire(&g_mouse_spinlock);

	if (kIsQueueEmpty(&(gs_stMouseQueue)) == TRUE)
	{
		SpinlockRelease(&g_mouse_spinlock);
		return FALSE;
	}

	bResult = kGetQueue(&(gs_stMouseQueue), &stData);
	SpinlockRelease(&g_mouse_spinlock);

    // �����͸� ������ �������� ����
    if( bResult == FALSE )
    {
        return FALSE;
    }
    
    // ���콺 ������ �м�
    // ���콺 ��ư �÷��״� ù ��° ����Ʈ�� ���� 3��Ʈ�� ������
    *pbButtonStatus = stData.bButtonStatusAndFlag & 0x7;

	if (stData.bAbsoluteCoordinate == true)
	{
		bAbsoluteCoordinate = true;
		*piRelativeX = stData.bXMovement;
		*piRelativeY = stData.bYMovement;
		return true;
	}

    // X, Y�� �̵��Ÿ� ����
    // X�� ��ȣ ��Ʈ�� ��Ʈ 4�� ������ 1�� �����Ǿ������� ������
    *piRelativeX = stData.bXMovement & 0xFF;
    if( stData.bButtonStatusAndFlag & 0x10 )
    {
        // �����̹Ƿ� �Ʒ� 8��Ʈ�� X �̵��Ÿ��� ������ �� ���� ��Ʈ�� ��� 1�� �����
        // ��ȣ ��Ʈ�� Ȯ���� 
        *piRelativeX |= ( 0xFFFFFF00 );
    }
    
    // Y�� ��ȣ ��Ʈ�� ��Ʈ 5�� ������, 1�� �����Ǿ����� ������
    // �Ʒ� �������� ������ Y ���� �����ϴ� ȭ�� ��ǥ�� �޸� ���콺�� ���� �������� ������
    // ���� �����ϹǷ� ����� ���� �� ��ȣ�� ������
    *piRelativeY = stData.bYMovement & 0xFF;
    if( stData.bButtonStatusAndFlag & 0x20 )
    {
        // �����̹Ƿ� �Ʒ� 8��Ʈ�� Y �̵��Ÿ��� ������ �� ���� ��Ʈ�� ��� 1�� �����
        // ��ȣ ��Ʈ�� Ȯ���� 
        *piRelativeY |= ( 0xFFFFFF00 );
    }

    // ���콺�� Y�� ���� ������ ȭ�� ��ǥ�� �ݴ��̹Ƿ� Y �̵��Ÿ��� -�Ͽ� ������ �ٲ�
    *piRelativeY = -*piRelativeY;
    return TRUE;
}