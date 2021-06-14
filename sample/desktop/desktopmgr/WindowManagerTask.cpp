#include "windef.h"
#include "memory.h"
#include "WindowManagerTask.h"
#include "Mouse.h"
#include <stdio.h>
#include <systemcall_impl.h>
#include <Keyboard.h>
#include <Window.h>
/**
 *  ������ �Ŵ��� �½�ũ
 */

void kStartWindowManager(LinearBufferInfo* bufferInfo)
{
    int iMouseX, iMouseY;
	kInitializeMouse();
	kInitializeKeyboard();	

    // GUI �ý����� �ʱ�ȭ
    kInitializeGUISystem(bufferInfo);
    
    // ���� ���콺 ��ġ�� Ŀ���� ���
    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );    
}

void kUpdate(bool isDirectBuffer)
{
	bool bMouseDataResult = false;
	bool bKeyDataResult;
	bool bEventQueueResult;
    WINDOWMANAGER* pstWindowManager;

	// ������ �Ŵ����� ��ȯ
	pstWindowManager = kGetWindowManager();

	// ������ �Ŵ��� �½�ũ ����
	if(isDirectBuffer == true)
	{
		while (1)
		{
			// ���콺 �����͸� ó��
			bMouseDataResult = kProcessMouseData();

			// Ű �����͸� ó��
			bKeyDataResult = kProcessKeyData();

			// ������ �Ŵ����� �̺�Ʈ ť�� ���ŵ� �����͸� ó��. ���ŵ� ��� �̺�Ʈ�� ó����
			bEventQueueResult = FALSE;
			while (kProcessEventQueueData() == TRUE)
			{
				kProcessEventQueueData();
				bEventQueueResult = TRUE;
			}

			// �̺�Ʈ ť�� �̺�Ʈ�� ó���ߴٸ� ������ ũ�� ���� ǥ���� ������ �� �����Ƿ� �ٽ� ���
			if ((bEventQueueResult == TRUE) &&
				(pstWindowManager->bWindowResizeMode == TRUE))
			{
				// ���ο� ��ġ�� ������ ũ�� ���� ǥ���� ǥ��
				kDrawResizeMarker(&(pstWindowManager->stResizingWindowArea), TRUE);
			}

			// ó���� �����Ͱ� �ϳ��� ���ٸ� Sleep()�� �����Ͽ� ���μ����� �纸��
			if ((bMouseDataResult == FALSE) &&
				(bKeyDataResult == FALSE) &&
				(bEventQueueResult == FALSE))
			{
				//20180628
				Syscall_Sleep(1);
			}
		}
	}
	else
	{
		// ���콺 �����͸� ó��
		bMouseDataResult = kProcessMouseData();

		// Ű �����͸� ó��
		bKeyDataResult = kProcessKeyData();

		// ������ �Ŵ����� �̺�Ʈ ť�� ���ŵ� �����͸� ó��. ���ŵ� ��� �̺�Ʈ�� ó����
		bEventQueueResult = FALSE;
		while (kProcessEventQueueData() == TRUE)
		{
			kProcessEventQueueData();
			bEventQueueResult = TRUE;
		}

		// �̺�Ʈ ť�� �̺�Ʈ�� ó���ߴٸ� ������ ũ�� ���� ǥ���� ������ �� �����Ƿ� �ٽ� ���
		if ((bEventQueueResult == TRUE) &&
			(pstWindowManager->bWindowResizeMode == TRUE))
		{
			// ���ο� ��ġ�� ������ ũ�� ���� ǥ���� ǥ��
			kDrawResizeMarker(&(pstWindowManager->stResizingWindowArea), TRUE);
		}

		// ó���� �����Ͱ� �ϳ��� ���ٸ� Sleep()�� �����Ͽ� ���μ����� �纸��
		if ((bMouseDataResult == FALSE) &&
			(bKeyDataResult == FALSE) &&
			(bEventQueueResult == FALSE))
		{
			//20180628
			 Syscall_Sleep( 0 );
		}
	}
}

/**
 *  ���ŵ� ���콺 �����͸� ó��
 */
bool kProcessMouseData( void )
{
    QWORD qwWindowIDUnderMouse;
    BYTE bButtonStatus;
    int iRelativeX, iRelativeY;
    int iMouseX, iMouseY;
	bool bAbsoluteCoordinate = false;
    int iPreviousMouseX, iPreviousMouseY;
    BYTE bChangedButton;
    RECT stWindowArea;
    EVENT stEvent;
    WINDOWMANAGER* pstWindowManager;
    int i;
    int iWidth, iHeight;
    
    // ������ �Ŵ����� ��ȯ
    pstWindowManager = kGetWindowManager();

    //--------------------------------------------------------------------------
    // ���콺 �̺�Ʈ�� �����ϴ� �κ�
    //--------------------------------------------------------------------------
    for( i = 0 ; i < WINDOWMANAGER_DATAACCUMULATECOUNT ; i++ )
    {
        // ���콺 �����Ͱ� ���ŵǱ⸦ ��ٸ�
        if( kGetMouseDataFromMouseQueue( &bButtonStatus, &iRelativeX, &iRelativeY, bAbsoluteCoordinate) ==
            FALSE )
        {
            // ó������ Ȯ���ߴµ� �����Ͱ� ���ٸ� ����
            if( i == 0 )
            {
                return FALSE;
            }
            // ó���� �ƴ� ���� ���� �������� ���콺 �̺�Ʈ�� ���������Ƿ� 
            // ������ �̺�Ʈ�� ó��
            else
            {
                break;
            }
        }
        
        // ���� ���콺 Ŀ�� ��ġ�� ��ȯ
        kGetCursorPosition( &iMouseX, &iMouseY );
        
        // ó�� ���콺 �̺�Ʈ�� ���ŵ� ���̸� ���� ��ǥ�� ���� ���콺�� ��ġ�� �����ص�
        if( i == 0 )
        {
            // �����̱� ������ ��ǥ�� ����
            iPreviousMouseX = iMouseX;
            iPreviousMouseY = iMouseY;
        }
        
		if (bAbsoluteCoordinate == true)
		{
			iMouseX = iRelativeX;
			iMouseY = iRelativeY;
		}
		else
		{
			// ���콺�� ������ �Ÿ��� ���� Ŀ�� ��ġ�� ���ؼ� ���� ��ǥ�� ���
			iMouseX += iRelativeX;
			iMouseY += iRelativeY;
		}        
        
        // ���ο� ��ġ�� ���콺 Ŀ���� �̵��ϰ� �ٽ� ���� Ŀ���� ��ġ�� ��ȯ
        // ���콺 Ŀ���� ȭ���� ����� �ʵ��� ó���� Ŀ�� ��ǥ�� ����Ͽ� ȭ���� ���
        // Ŀ���� ���� �߻��ϴ� ������ ����
        kMoveCursor( iMouseX, iMouseY );
        kGetCursorPosition( &iMouseX, &iMouseY );
        
        // ��ư ���´� ���� ��ư ���¿� ���� ��ư ���¸� XOR�Ͽ� 1�� �����ƴ����� Ȯ��
        bChangedButton = pstWindowManager->bPreviousButtonStatus ^ bButtonStatus;

        // ���콺�� ���������� ��ư�� ��ȭ�� �ִٸ� �ٷ� �̺�Ʈ ó��
        if( bChangedButton != 0 )
        {
            break;
        }
    }
    
    //--------------------------------------------------------------------------
    // ���콺 �̺�Ʈ�� ó���ϴ� �κ�
    //--------------------------------------------------------------------------
    // ���� ���콺 Ŀ�� �Ʒ��� �ִ� �����츦 �˻�
    qwWindowIDUnderMouse = kFindWindowByPoint( iMouseX, iMouseY );
    
    //--------------------------------------------------------------------------
    // ��ư ���°� ���ߴ��� Ȯ���ϰ� ��ư ���¿� ���� ���콺 �޽����� ������ �޽�����
    // ����
    //--------------------------------------------------------------------------
    // ���콺 ���� ��ư�� ��ȭ�� ���� ��� ó��
    if( bChangedButton & MOUSE_LBUTTONDOWN )
    {
        // ���� ��ư�� ���� ��� ó��
        if( bButtonStatus & MOUSE_LBUTTONDOWN )
        {
            // ���콺�� �����츦 ������ ���̹Ƿ�, ���콺 �Ʒ��� �ִ� �����찡
            // ��� �����찡 �ƴ� ��� �ֻ����� �÷���
            if( qwWindowIDUnderMouse != pstWindowManager->qwBackgroundWindowID )
            {
                // ���õ� �����츦 �ֻ����� ����
                // �����츦 �ֻ����� ����鼭 ���ÿ� ������ ���ð� ���� ���� �̺�Ʈ��
                // ���� ����
                kMoveWindowToTop( qwWindowIDUnderMouse );
            }
            
            //------------------------------------------------------------------
            // ���� ��ư�� ���� ��ġ�� ���� ǥ������ ��ġ�̸� ������ �̵����� �Ǵ�
            // �ݱ� ��ư ��ġ���� ���ȴ��� Ȯ���Ͽ� ó��
            //------------------------------------------------------------------
            if( kIsInTitleBar( qwWindowIDUnderMouse, iMouseX, iMouseY ) == TRUE )
            {
                // �ݱ� ��ư���� �������� �����쿡 �ݱ� ����
                if( kIsInCloseButton( qwWindowIDUnderMouse, iMouseX, iMouseY ) 
                        == TRUE )
                {
                    // ������ �ݱ� �̺�Ʈ�� ����
                    kSetWindowEvent( qwWindowIDUnderMouse, EVENT_WINDOW_CLOSE,
                            &stEvent );
                    kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
                }
                // ������ ũ�� ���� ��ư���� �������� ũ�� ���� ���� ����
                else if( kIsInResizeButton( qwWindowIDUnderMouse, iMouseX, iMouseY ) 
                           == TRUE )
                {
                    // ������ ũ�� ���� ��� ����
                    pstWindowManager->bWindowResizeMode = TRUE;
                    
                    // ���� �����츦 ũ�� ���� ������� ����
                    pstWindowManager->qwResizingWindowID = qwWindowIDUnderMouse;
                    
                    // ���� �������� ũ�⸦ ����
                    kGetWindowArea( qwWindowIDUnderMouse, 
                            &( pstWindowManager->stResizingWindowArea ) );
                    
                    // ������ ũ�� ���� ǥ���� ǥ��
                    kDrawResizeMarker( &( pstWindowManager->stResizingWindowArea ), 
                            TRUE );
                }
                // �ݱ� ��ư�� �ƴϸ� ������ �̵� ���� ����
                else
                {
                    // ������ �̵� ��� ����
                    pstWindowManager->bWindowMoveMode = TRUE;
                    
                    // ���� �����츦 �̵��ϴ� ������� ����
                    pstWindowManager->qwMovingWindowID = qwWindowIDUnderMouse;
                }
            }
            // ������ ���ο��� ���� ���̸� ���� ��ư�� ���������� ����
            else
            {
                // ���� ��ư ���� �̺�Ʈ�� ����
                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONDOWN,
                        iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            }
        }
        // ���� ��ư�� ������ ��� ó��
        else
        {
            // �����찡 �̵� ���̾����� ��常 ����
            if( pstWindowManager->bWindowMoveMode == TRUE )
            {
                // �̵� ���̶�� �÷��׸� ����
                pstWindowManager->bWindowMoveMode = FALSE;
                pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;
            }
            // �����찡 ũ�� ���� ���̾����� ������ ũ��� ������ ũ�� ����
            else if( pstWindowManager->bWindowResizeMode == TRUE )
            {
                // ������ �Ŵ��� �ڷᱸ���� ����� ������ �̿��Ͽ� �������� ũ�⸦ ����
                iWidth = kGetRectangleWidth( &( pstWindowManager->stResizingWindowArea ) );
                iHeight = kGetRectangleHeight( &( pstWindowManager->stResizingWindowArea ) );
                kResizeWindow( pstWindowManager->qwResizingWindowID, 
                        pstWindowManager->stResizingWindowArea.left,
                        pstWindowManager->stResizingWindowArea.top,
                        iWidth, iHeight );
                
                // ������ ũ�� ���� ǥ�� ����
                kDrawResizeMarker( &( pstWindowManager->stResizingWindowArea ), FALSE );

                // ������� ũ�� ���� �̺�Ʈ ����
                kSetWindowEvent( pstWindowManager->qwResizingWindowID, EVENT_WINDOW_RESIZE, 
                        &stEvent );
                kSendEventToWindow( pstWindowManager->qwResizingWindowID, &stEvent );
                
                // ũ�� ���� ���̶�� �÷��׸� ����
                pstWindowManager->bWindowResizeMode = FALSE;
                pstWindowManager->qwResizingWindowID = WINDOW_INVALIDID;
            }
            // �����찡 �̵� ���� �ƴϾ����� ������� ���� ��ư ������ �̺�Ʈ�� ����
            else
            {
                // ���� ��ư ������ �̺�Ʈ�� ����
                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONUP,
                        iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            }
        }
    }            
    // ���콺 ������ ��ư�� ��ȭ�� ���� ��� ó��
    else if( bChangedButton & MOUSE_RBUTTONDOWN )
    {
        // ������ ��ư�� ���� ��� ó��
        if( bButtonStatus & MOUSE_RBUTTONDOWN )
        {
            // ������ ��ư ���� �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONDOWN,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
        else
        {
            // ������ ��ư ������ �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONUP,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
    }
    // ���콺 ��� ��ư�� ��ȭ�� ���� ��� ó��
    else if( bChangedButton & MOUSE_MBUTTONDOWN )
    {
        // ��� ��ư�� ���� ��� ó��
        if( bButtonStatus & MOUSE_MBUTTONDOWN )
        {
            // ��� ��ư ���� �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONDOWN,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
        else
        {
            // ��� ��ư ������ �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONUP,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
    }        
    // ���콺 ��ư�� ������� �ʾ����� ���콺 �̵� ó���� ����
    else
    {
        // ���콺 �̵� �̺�Ʈ�� ����
        kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MOVE, iMouseX, iMouseY, 
                    bButtonStatus, &stEvent );
        kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
    }

    //--------------------------------------------------------------------------
    //  ������ �̵��� ũ�� ���� ó��
    //--------------------------------------------------------------------------    
    // �����찡 �̵� ���̾����� ������ �̵� ó��
    if( pstWindowManager->bWindowMoveMode == TRUE )
    {
        // �������� ��ġ�� ����
        if( kGetWindowArea( pstWindowManager->qwMovingWindowID, &stWindowArea )
                == TRUE )
        {
            // �������� ���� ��ġ�� �� ���콺�� �̵��� ��ŭ �Ű���
            // ������ �̵� �̺�Ʈ�� �Լ� ���ο��� ����
            kMoveWindow( pstWindowManager->qwMovingWindowID, 
                         stWindowArea.left + iMouseX - iPreviousMouseX,
                         stWindowArea.top + iMouseY - iPreviousMouseY );
        }
        // �������� ��ġ�� ���� �� ������ �����찡 �������� �ʴ� ���̹Ƿ�
        // ������ �̵� ��� ����
        else
        {
            // �̵� ���̶�� �÷��׸� ����
            pstWindowManager->bWindowMoveMode = FALSE;
            pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;
        }
    }
    // �����찡 ũ�� ���� ���̾����� ������ ũ�� ���� ó��
    else if( pstWindowManager->bWindowResizeMode == TRUE )
    {
        // ���� ��ġ�� ������ ũ�� ���� ǥ���� ����
        kDrawResizeMarker( &( pstWindowManager->stResizingWindowArea ), FALSE );
        
        // ���콺 �̵� �Ÿ��� �̿��Ͽ� ���ο� ������ ũ�⸦ ����
        pstWindowManager->stResizingWindowArea.right += iMouseX - iPreviousMouseX;
        pstWindowManager->stResizingWindowArea.top += iMouseY - iPreviousMouseY;

        // �������� ũ�Ⱑ �ּ� �� �����̸� �ּڰ����� �ٽ� ����
        if( ( pstWindowManager->stResizingWindowArea.right < pstWindowManager->stResizingWindowArea.left) ||
            ( kGetRectangleWidth( &( pstWindowManager->stResizingWindowArea ) ) < WINDOW_WIDTH_MIN ) )
        {
            pstWindowManager->stResizingWindowArea.right =
                pstWindowManager->stResizingWindowArea.left + WINDOW_WIDTH_MIN - 1;
        }        
        if( ( pstWindowManager->stResizingWindowArea.bottom < pstWindowManager->stResizingWindowArea.top) ||
            ( kGetRectangleHeight( &( pstWindowManager->stResizingWindowArea ) ) < WINDOW_HEIGHT_MIN ) )
        {
            pstWindowManager->stResizingWindowArea.top =
                pstWindowManager->stResizingWindowArea.bottom - WINDOW_HEIGHT_MIN - 1;
        }
        
        // ���ο� ��ġ�� ������ ũ�� ���� ǥ���� ���
        kDrawResizeMarker( &( pstWindowManager->stResizingWindowArea ), TRUE );
    }
    
    // ���� ó���� ����Ϸ��� ���� ��ư ���¸� ������
    pstWindowManager->bPreviousButtonStatus = bButtonStatus;
    return TRUE;
}


/**
 *  ���ŵ� Ű �����͸� ó��
 */
bool kProcessKeyData( void )
{
    KEYDATA stKeyData;
    EVENT stEvent;
    QWORD qwAcitveWindowID;
    
    // Ű���� �����Ͱ� ���ŵǱ⸦ ��ٸ�
    if( kGetKeyFromKeyQueue( &stKeyData ) == FALSE )
    {
        return FALSE;
    }
    
	//SendSerialLog("Key %x, %x, %x\n", stKeyData.bASCIICode, stKeyData.bScanCode, stKeyData.bFlags);

    // �ֻ��� ������, �� ���õ� ������� �޽����� ����
    qwAcitveWindowID = kGetTopWindowID();
    kSetKeyEvent( qwAcitveWindowID, &stKeyData, &stEvent );

    WINDOW* pstWindow = kGetWindowWithWindowLock(qwAcitveWindowID);
    if (pstWindow != NULL)
    {
        
        kPutQueue(&pstWindow->stKeyEventQueue, &stEvent);

        SpinlockRelease(&pstWindow->stLock);
    }

    return kSendEventToWindow( qwAcitveWindowID, &stEvent );
}    

/**
 *  �̺�Ʈ ť�� ���ŵ� �̺�Ʈ ó��
 */
EVENT vstEvent[WINDOWMANAGER_DATAACCUMULATECOUNT];
bool kProcessEventQueueData(void)
{
	int iEventCount;
	WINDOWEVENT* pstWindowEvent;
	WINDOWEVENT* pstNextWindowEvent;
	RECT stArea;
	RECT stOverlappedArea;
	int i;
	int j;

	//--------------------------------------------------------------------------
	// ������ �Ŵ��� �½�ũ�� �̺�Ʈ ť�� ���ŵ� �̺�Ʈ�� �����ϴ� �κ�
	//--------------------------------------------------------------------------
	for (i = 0; i < WINDOWMANAGER_DATAACCUMULATECOUNT; i++)
	{
		// �̺�Ʈ�� ���ŵǱ⸦ ��ٸ�
		if (kReceiveEventFromWindowManagerQueue(&(vstEvent[i])) == FALSE)
		{
			// ó������ �̺�Ʈ�� ���ŵ��� �ʾ����� ����
			if (i == 0)
			{
				return FALSE;
			}
			else
			{
				break;
			}
		}

        if (vstEvent[i].qwType == EVENT_CONSOLE_PRINT)
        {
            WINDOW* pstWindow;

            // ������ �˻��� ����ȭ ó��
            pstWindow = kGetWindowWithWindowLock(vstEvent[i].stKeyEvent.qwWindowID);
            if (pstWindow == NULL)
            {
                continue;
            }

            kPutQueue(&pstWindow->stKeyEventQueue, &vstEvent[i]);


            SpinlockRelease(&pstWindow->stLock);
            continue;
        }

		pstWindowEvent = &(vstEvent[i].stWindowEvent);
		// ������ ID�� ������Ʈ�ϴ� �̺�Ʈ�� ���ŵǸ� ������ ������ �̺�Ʈ �����Ϳ� ����
		if (vstEvent[i].qwType == EVENT_WINDOWMANAGER_UPDATESCREENBYID)
		{
			// �������� ũ�⸦ �̺�Ʈ �ڷᱸ���� ����
			if (kGetWindowArea(pstWindowEvent->qwWindowID, &stArea) == FALSE)
			{
				kSetRectangleData(0, 0, 0, 0, &(pstWindowEvent->stArea));
			}
			else
			{
				kSetRectangleData(0, 0, kGetRectangleWidth(&stArea) - 1,
					kGetRectangleHeight(&stArea) - 1, &(pstWindowEvent->stArea));
			}
		}
	}

	// ����� �̺�Ʈ�� �˻��ϸ鼭 ��ĥ �� �ִ� �̺�Ʈ�� �ϳ��� ����
	iEventCount = i;

	for (j = 0; j < iEventCount; j++)
	{
		// ���ŵ� �̺�Ʈ �߿� �̺�Ʈ �߿��� �̹��� ó���� �Ͱ� ���� �����쿡��
		// �߻��ϴ� ������ �̺�Ʈ�� �˻�
		pstWindowEvent = &(vstEvent[j].stWindowEvent);
		if ((vstEvent[j].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYID) &&
			(vstEvent[j].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA) &&
			(vstEvent[j].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA))
		{
			continue;
		}

		// ������ �̺�Ʈ�� ������ ������ �����ϸ鼭 ���ŵ� �̺�Ʈ�� �˻�
		for (i = j + 1; i < iEventCount; i++)
		{
			pstNextWindowEvent = &(vstEvent[i].stWindowEvent);
			// ȭ�� ������Ʈ�� �ƴϰų� ������ ID�� ��ġ���� ������ ����
			if (((vstEvent[i].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYID) &&
				(vstEvent[i].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA) &&
				(vstEvent[i].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA)) ||
				(pstWindowEvent->qwWindowID != pstNextWindowEvent->qwWindowID))
			{
				continue;
			}

			// ��ġ�� ������ ����Ͽ� ��ġ�� ������ ����
			if (kGetOverlappedRectangle(&(pstWindowEvent->stArea),
				&(pstNextWindowEvent->stArea), &stOverlappedArea) == FALSE)
			{
				continue;
			}

			// �� ������ ��ġ�ϰų� ��� ������ ���ԵǸ� �̺�Ʈ�� ����
			if (memcmp(&(pstWindowEvent->stArea), &stOverlappedArea,
				sizeof(RECT)) == 0)
			{
				// ���� �̺�Ʈ�� �������� ������ ��ġ�� ������ ��ġ�Ѵٸ� 
				// ���� �̺�Ʈ�� ������ ������ ���� ������ ������ ���ų� ������
				// ���� ���� �̺�Ʈ�� ���� �̺�Ʈ�� ������ ������ �����ϰ�
				// ���� �̺�Ʈ�� ����
				memcpy(&(pstWindowEvent->stArea), &(pstNextWindowEvent->stArea),
					sizeof(RECT));
				vstEvent[i].qwType = EVENT_UNKNOWN;
			}
			else if (memcmp(&(pstNextWindowEvent->stArea), &stOverlappedArea,
				sizeof(RECT)) == 0)
			{
				// ���� �̺�Ʈ�� �������� ������ ��ġ�� ������ ��ġ�Ѵٸ� 
				// ���� �̺�Ʈ�� ������ ������ ���� ������ ������ ���ų� ������
				// ���� ������ ������ �������� �ʰ� ���� �̺�Ʈ�� ����
				vstEvent[i].qwType = EVENT_UNKNOWN;
			}
		}
	}

	// ���յ� �̺�Ʈ�� ��� ó��
	for (i = 0; i < iEventCount; i++)
	{
		pstWindowEvent = &(vstEvent[i].stWindowEvent);

		// Ÿ�� ���� ó��
		switch (vstEvent[i].qwType)
		{
			// ���� �����찡 �ִ� ������ ȭ�鿡 ������Ʈ
		case EVENT_WINDOWMANAGER_UPDATESCREENBYID:
			// �������� ���� ������ ȭ�鿡 ������Ʈ
		case EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA:
			// �����츦 �������� �� ��ǥ�� ȭ�� ��ǥ�� ��ȯ�Ͽ� ������Ʈ ó��
			if (kConvertRectClientToScreen(pstWindowEvent->qwWindowID,
				&(pstWindowEvent->stArea), &stArea) == TRUE)
			{
				// ������ ������ ������ �����Ƿ� �״�� ȭ�� ������Ʈ �Լ��� ȣ��
				kRedrawWindowByArea(&stArea, pstWindowEvent->qwWindowID);
			}
			break;

			// ȭ�� ��ǥ�� ���޵� ������ ȭ�鿡 ������Ʈ
		case EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA:
			kRedrawWindowByArea(&(pstWindowEvent->stArea), WINDOW_INVALIDID);
			break;

		default:
			break;
		}
	}

	return TRUE;
}

/**
 *  ���� �޸𸮿� ������ ũ�� ���� ǥ���� ����ϰų� ��µ� ǥ���� ����
 */
void kDrawResizeMarker( const RECT* pstArea, bool bShowMarker )
{
    RECT stMarkerArea;
    WINDOWMANAGER* pstWindowManager;
    
    // ������ �Ŵ����� ��ȯ
    pstWindowManager = kGetWindowManager();
    
    // ������ ũ�� ���� ǥ���� ����ϴ� ���
    if( bShowMarker == TRUE )
    {
        // ���� �� ǥ��
        kSetRectangleData( pstArea->left, pstArea->top,
            pstArea->left + WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->top + WINDOWMANAGER_RESIZEMARKERSIZE, &stMarkerArea );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.left, stMarkerArea.top,
                stMarkerArea.right, stMarkerArea.top + WINDOWMANAGER_THICK_RESIZEMARKER,
                WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.left, stMarkerArea.top,
                stMarkerArea.left + WINDOWMANAGER_THICK_RESIZEMARKER, stMarkerArea.bottom,
                WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );
        
        // ������ �� ǥ��
        kSetRectangleData( pstArea->right - WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->top, pstArea->right, pstArea->top + WINDOWMANAGER_RESIZEMARKERSIZE,
            &stMarkerArea );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.left, stMarkerArea.top,
                stMarkerArea.right, stMarkerArea.top + WINDOWMANAGER_THICK_RESIZEMARKER,
                WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.right -
                WINDOWMANAGER_THICK_RESIZEMARKER, stMarkerArea.top,
                stMarkerArea.right, stMarkerArea.bottom, WINDOWMANAGER_COLOR_RESIZEMARKER,
                TRUE );

        // ���� �Ʒ� ǥ��
        kSetRectangleData( pstArea->left, pstArea->bottom - WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->left + WINDOWMANAGER_RESIZEMARKERSIZE, pstArea->bottom, &stMarkerArea );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.left, stMarkerArea.bottom -
                WINDOWMANAGER_THICK_RESIZEMARKER, stMarkerArea.right, stMarkerArea.bottom,
                WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.left, stMarkerArea.top,
                stMarkerArea.left + WINDOWMANAGER_THICK_RESIZEMARKER, stMarkerArea.bottom,
                WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );

        // ������ �Ʒ� ǥ��
        kSetRectangleData( pstArea->right - WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->bottom - WINDOWMANAGER_RESIZEMARKERSIZE, pstArea->right, pstArea->bottom,
            &stMarkerArea );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.left, stMarkerArea.bottom -
                WINDOWMANAGER_THICK_RESIZEMARKER, stMarkerArea.right, stMarkerArea.bottom,
                WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );
        kInternalDrawRect( &( pstWindowManager->stScreenArea ), 
                pstWindowManager->pstVideoMemory, stMarkerArea.right -
                WINDOWMANAGER_THICK_RESIZEMARKER, stMarkerArea.top, stMarkerArea.right,
                stMarkerArea.bottom, WINDOWMANAGER_COLOR_RESIZEMARKER, TRUE );
    }
    // ������ ũ�� ���� ǥ���� �����ϴ� ���
    else
    {
        // ũ�� ���� ǥ���� ������ �� �𼭸��� �����Ƿ� �𼭸��� ȭ�� ������Ʈ
        // ���� �� ǥ��
        kSetRectangleData( pstArea->left, pstArea->top,
            pstArea->left + WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->top + WINDOWMANAGER_RESIZEMARKERSIZE, &stMarkerArea );
        kRedrawWindowByArea( &stMarkerArea, WINDOW_INVALIDID );
        
        // ������ �� ǥ��
        kSetRectangleData( pstArea->right - WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->top, pstArea->right, pstArea->top + WINDOWMANAGER_RESIZEMARKERSIZE,
            &stMarkerArea );
        kRedrawWindowByArea( &stMarkerArea, WINDOW_INVALIDID );

        // ���� �Ʒ� ǥ��
        kSetRectangleData( pstArea->left, pstArea->bottom - WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->left + WINDOWMANAGER_RESIZEMARKERSIZE, pstArea->bottom, &stMarkerArea );
        kRedrawWindowByArea( &stMarkerArea, WINDOW_INVALIDID );

        // ������ �Ʒ� ǥ��
        kSetRectangleData( pstArea->right - WINDOWMANAGER_RESIZEMARKERSIZE,
            pstArea->bottom - WINDOWMANAGER_RESIZEMARKERSIZE, pstArea->right, pstArea->bottom,
            &stMarkerArea );
        kRedrawWindowByArea( &stMarkerArea, WINDOW_INVALIDID );
    }
}