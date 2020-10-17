#pragma once

#include "windef.h"
#include "skyoswindow.h"
#include "2DGraphics.h"
#include <InputList.h>
#include <InputQueue.h>
#include <Keyboard.h>
#include <_spinlock.h>

// 윈도우의 정보를 저장하는 자료구조
typedef struct kWindowStruct
{
    // 다음 데이터의 위치와 현재 윈도우의 ID
    LISTLINK stLink;

    // 자료구조 동기화를 위한 스핀락
	Spinlock_t stLock;

    // 윈도우 영역 정보
    RECT stArea;

    // 윈도우의 화면 버퍼 어드레스
    COLOR* pstWindowBuffer;

    // 윈도우를 가지고 있는 태스크의 ID
    QWORD qwTaskID;

    // 윈도우 속성
    DWORD dwFlags;

    // 이벤트 큐와 큐에서 사용할 버퍼
    QUEUE stKeyEventQueue;
    QUEUE stEventQueue;
    EVENT* pstEventBuffer;
    EVENT* pstKeyEventBuffer;
    
    // 윈도우 제목
    char vcWindowTitle[ WINDOW_TITLEMAXLENGTH + 1 ];
} WINDOW;

// 윈도우 풀의 상태를 관리하는 자료구조
typedef struct kWindowPoolManagerStruct
{
    // 자료구조 동기화를 위한 스핀락
	Spinlock_t stLock;

    // 윈도우 풀에 대한 정보
    WINDOW* pstStartAddress;
    int iMaxCount;
    int iUseCount;

    // 윈도우가 할당된 횟수
    int iAllocatedCount;
} WINDOWPOOLMANAGER;

// 윈도우 매니저 자료구조
typedef struct kWindowManagerStruct
{
    // 자료구조 동기화를 위한 스핀락
	Spinlock_t stLock;

    // 윈도우 리스트
    LIST stWindowList;

    // 현재 마우스 커서의 X, Y좌표
    int iMouseX;
    int iMouseY;

    // 화면 영역 정보
    RECT stScreenArea;

    // 비디오 메모리의 어드레스
    COLOR* pstVideoMemory;

    // 배경 윈도우의 ID
    QWORD qwBackgroundWindowID;
    
    // 이벤트 큐와 큐에서 사용할 버퍼
    QUEUE stEventQueue;
    EVENT* pstEventBuffer;

    // 마우스 버튼의 이전 상태
    BYTE bPreviousButtonStatus;
    
    // 이동 중인 윈도우의 ID와 윈도우 이동 모드
    QWORD qwMovingWindowID;
	bool bWindowMoveMode;
    
    // 윈도우 크기 변경 모드와 크기 변경 중인 윈도우의 ID, 변경 중인 윈도우의 영역
    bool bWindowResizeMode;    
    QWORD qwResizingWindowID;
    RECT stResizingWindowArea;
    
    // 화면 업데이트용 비트맵 버퍼의 어드레스
    BYTE* pbDrawBitmap;
} WINDOWMANAGER;


// 화면에 업데이트할 영역의 비트맵 정보을 저장하는 자료구조
typedef struct kDrawBitmapStruct
{
    // 업데이트할 화면 영역
    RECT stArea;
    // 화면 영역의 정보가 저장된 비트맵의 어드레스
    BYTE* pbBitmap;
} DRAWBITMAP;

// 윈도우 풀 관련
static void kInitializeWindowPool( void );
static WINDOW* kAllocateWindow( void );
static void kFreeWindow( QWORD qwID );

// 윈도우와 윈도우 매니저 관련
void kInitializeGUISystem(LinearBufferInfo* bufferInfo);
WINDOWMANAGER* kGetWindowManager( void );
QWORD kGetBackgroundWindowID( void );
void kGetScreenArea( RECT* pstScreenArea );
QWORD kCreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags, const char* pcTitle );
QWORD kCreateSkyWindow(int iX, int iY, int iWidth, int iHeight, DWORD dwFlags, const char* pcTitle);
bool kDeleteWindow( QWORD qwWindowID );
bool kDeleteAllWindowInTaskID( QWORD qwTaskID );
WINDOW* kGetWindow( QWORD qwWindowID );
WINDOW* kGetWindowWithWindowLock( QWORD qwWindowID );
bool kShowWindow( QWORD qwWindowID, bool bShow );
bool kRedrawWindowByArea( const RECT* pstArea, QWORD qwDrawWindowID );
static void kCopyWindowBufferToFrameBuffer( const WINDOW* pstWindow,
        DRAWBITMAP* pstDrawBitmap );
QWORD kFindWindowByPoint( int iX, int iY );
QWORD kFindWindowByTitle( const char* pcTitle );
bool kIsWindowExist( QWORD qwWindowID );
QWORD kGetTopWindowID( void );
bool kMoveWindowToTop( QWORD qwWindowID );
bool kIsInTitleBar( QWORD qwWindowID, int iX, int iY );
bool kIsInCloseButton( QWORD qwWindowID, int iX, int iY );
bool kMoveWindow( QWORD qwWindowID, int iX, int iY );
static bool kUpdateWindowTitle( QWORD qwWindowID, bool bSelectedTitle );
bool kResizeWindow( QWORD qwWindowID, int iX, int iY, int iWidth, int iHeight );
bool kIsInResizeButton(QWORD qwWindowID, int iX, int iY);

// 좌표 변환 관련
bool kGetWindowArea( QWORD qwWindowID, RECT* pstArea );
bool kConvertPointScreenToClient( QWORD qwWindowID, const POINT* pstXY,
        POINT* pstXYInWindow );
bool kConvertPointClientToScreen( QWORD qwWindowID, const POINT* pstXY,
        POINT* pstXYInScreen );
bool kConvertRectScreenToClient( QWORD qwWindowID, const RECT* pstArea,
        RECT* pstAreaInWindow );
bool kConvertRectClientToScreen( QWORD qwWindowID, const RECT* pstArea,
        RECT* pstAreaInScreen );

// 화면 업데이트 관련
bool kUpdateScreenByID( QWORD qwWindowID );
bool kUpdateScreenByWindowArea( QWORD qwWindowID, const RECT* pstArea );
bool kUpdateScreenByScreenArea( const RECT* pstArea );

//  이벤트 큐 관련
bool kSendEventToWindow( QWORD qwWindowID, const EVENT* pstEvent );
bool kReceiveEventFromWindowQueue( QWORD qwWindowID, EVENT* pstEvent );
char kGetKeyFromWindowId(QWORD qwWindowID);

bool kSendEventToWindowManager( const EVENT* pstEvent );
bool kReceiveEventFromWindowManagerQueue( EVENT* pstEvent );
bool kSetMouseEvent( QWORD qwWindowID, QWORD qwEventType, int iMouseX, int iMouseY,
        BYTE bButtonStatus, EVENT* pstEvent );
bool kSetWindowEvent( QWORD qwWindowID, QWORD qwEventType, EVENT* pstEvent );
void kSetKeyEvent( QWORD qwWindow, const KEYDATA* pstKeyData, EVENT* pstEvent );

// 윈도우 내부에 그리는 함수와 마우스 커서 관련
bool kDrawWindowFrame( QWORD qwWindowID );
bool kDrawWindowBackground( QWORD qwWindowID );
bool kDrawWindowTitle( QWORD qwWindowID, const char* pcTitle, bool bSelectedTitle );
bool kDrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor,
        const char* pcText, COLOR stTextColor );
bool kDrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor );
bool kDrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor );
bool kDrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2,
        COLOR stColor, bool bFill );
bool kDrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor,
	bool bFill );
bool kDrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor,
        COLOR stBackgroundColor, const char* pcString, int iLength );
static void kDrawCursor( int iX, int iY );
void kMoveCursor( int iX, int iY );
void kGetCursorPosition( int* piX, int* piY );
bool kBitBlt( QWORD qwWindowID, int iX, int iY, COLOR* pstBuffer, int iWidth,
        int iHeight );
bool kBitBltWithRect(QWORD qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height);
void kDrawBackgroundImage( void );

//  화면 업데이트에 사용하는 화면 업데이트 비트맵 관련
bool kCreateDrawBitmap( const RECT* pstArea, DRAWBITMAP* pstDrawBitmap );
static bool kFillDrawBitmap( DRAWBITMAP* pstDrawBitmap, RECT* pstArea, bool bFill );
inline bool kGetStartPositionInDrawBitmap( const DRAWBITMAP* pstDrawBitmap,
        int iX, int iY, int* piByteOffset, int* piBitOffset );
inline bool kIsDrawBitmapAllOff( const DRAWBITMAP* pstDrawBitmap );
