#pragma once
#include <minwindef.h>

// 윈도우를 생성할 수 있는 최대 개수
#define WINDOW_MAXCOUNT             2048
// 윈도우 ID로 윈도우 풀 내의 오프셋을 계산하는 매크로
// 하위 32비트가 풀 내의 오프셋을 나타냄
#define GETWINDOWOFFSET( x )        ( ( x ) & 0xFFFFFFFF )
// 윈도우 제목의 최대 길이
#define WINDOW_TITLEMAXLENGTH       40
// 유효하지 않은 윈도우 ID
#define WINDOW_INVALIDID            0xFFFFFFFFFFFFFFFF

// 윈도우의 속성
// 윈도우를 화면에 나타냄
#define WINDOW_FLAGS_SHOW               0x00000001
// 윈도우 테두리 그림
#define WINDOW_FLAGS_DRAWFRAME          0x00000002
// 윈도우 제목 표시줄 그림
#define WINDOW_FLAGS_DRAWTITLE          0x00000004
// 윈도우 크기 변경 버튼을 그림
#define WINDOW_FLAGS_RESIZABLE          0x00000008
// 윈도우 기본 속성, 제목 표시줄과 프레임을 모두 그리고 화면에 윈도우를 보이게 설정
#define WINDOW_FLAGS_DEFAULT        ( WINDOW_FLAGS_SHOW | WINDOW_FLAGS_DRAWFRAME | \
                                      WINDOW_FLAGS_DRAWTITLE )

// 제목 표시줄의 높이
#define WINDOW_TITLEBAR_HEIGHT      21
// 윈도우의 닫기 버튼의 크기
#define WINDOW_XBUTTON_SIZE         19
// 윈도우의 최소 너비, 버튼 2개의 너비에 30픽셀의 여유 공간 확보
#define WINDOW_WIDTH_MIN            ( WINDOW_XBUTTON_SIZE * 2 + 30 )
// 윈도우의 최소 높이, 제목 표시줄의 높이에 30픽셀의 여유 공간 확보
#define WINDOW_HEIGHT_MIN           ( WINDOW_TITLEBAR_HEIGHT + 30 )

// 윈도우의 색깔
#define WINDOW_COLOR_FRAME                      RGB( 109, 218, 22 )
#define WINDOW_COLOR_BACKGROUND                 RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARTEXT               RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARACTIVEBACKGROUND   RGB( 79, 204, 11 )
#define WINDOW_COLOR_TITLEBARINACTIVEBACKGROUND RGB( 55, 135, 11 )
#define WINDOW_COLOR_TITLEBARBRIGHT1            RGB( 183, 249, 171 )
#define WINDOW_COLOR_TITLEBARBRIGHT2            RGB( 150, 210, 140 )
#define WINDOW_COLOR_TITLEBARUNDERLINE          RGB( 46, 59, 30 )
#define WINDOW_COLOR_BUTTONBRIGHT               RGB( 229, 229, 229 )
#define WINDOW_COLOR_BUTTONDARK                 RGB( 86, 86, 86 )
#define WINDOW_COLOR_SYSTEMBACKGROUND           RGB( 232, 255, 232 )
#define WINDOW_COLOR_XBUTTONLINECOLOR           RGB( 71, 199, 21 )

// 배경 윈도우의 제목
#define WINDOW_BACKGROUNDWINDOWTITLE            "SYS_BACKGROUND"


// 마우스 커서의 너비와 높이
#define MOUSE_CURSOR_WIDTH                  20
#define MOUSE_CURSOR_HEIGHT                 20

// 커서 이미지의 색깔
#define MOUSE_CURSOR_OUTERLINE              RGB(0, 0, 0 )
#define MOUSE_CURSOR_OUTER                  RGB( 79, 204, 11 )
#define MOUSE_CURSOR_INNER                  RGB( 232, 255, 232 )

// 이벤트 큐의 크기
#define EVENTQUEUE_WINDOWMAXCOUNT           100
#define EVENTQUEUE_WNIDOWMANAGERMAXCOUNT    WINDOW_MAXCOUNT

// 윈도우와 윈도우 매니저 태스크 사이에서 전달되는 이벤트의 종류
// 마우스 이벤트
#define EVENT_UNKNOWN                                   0
#define EVENT_MOUSE_MOVE                                1
#define EVENT_MOUSE_LBUTTONDOWN                         2
#define EVENT_MOUSE_LBUTTONUP                           3
#define EVENT_MOUSE_RBUTTONDOWN                         4
#define EVENT_MOUSE_RBUTTONUP                           5
#define EVENT_MOUSE_MBUTTONDOWN                         6
#define EVENT_MOUSE_MBUTTONUP                           7
// 윈도우 이벤트
#define EVENT_WINDOW_SELECT                             8
#define EVENT_WINDOW_DESELECT                           9
#define EVENT_WINDOW_MOVE                               10
#define EVENT_WINDOW_RESIZE                             11
#define EVENT_WINDOW_CLOSE                              12
// 키 이벤트
#define EVENT_KEY_DOWN                                  13
#define EVENT_KEY_UP                                    14
// 화면 업데이트 이벤트
#define EVENT_WINDOWMANAGER_UPDATESCREENBYID            15
#define EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA    16
#define EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA    17

// 화면에 업데이트할 때 이전에 업데이트한 영역을 저장해둘 개수
#define WINDOW_OVERLAPPEDAREALOGMAXCOUNT                20

#define EVENT_CONSOLE_PRINT							    21
#define EVENT_CONSOLE_COMMAND_END						22
#define EVENT_CONSOLE_KEY								23


typedef DWORD                COLOR;

#pragma pack(push, 1)

#ifdef WIN32STUB
typedef long long			QWORD;
#else
//32비트 처리 ARGB
#define ARGB(a, r, g, b)  (( BYTE )( r ) << 24) | (( BYTE )( r ) << 16) | (( BYTE )( g ) << 8 ) | (( BYTE )( b ) << 0 )
#define RGB(r, g, b)  (( BYTE )( r ) << 16) | (( BYTE )( g ) << 8 ) | (( BYTE )( b ) << 0 )

//16비트 처리
// 0~255 범위의 R, G, B를 16비트 색 형식으로 변환하는 매크로
// 0~255의 범위를 0~31, 0~63으로 축소하여 사용하므로 각각 8과 4로 나누어줘야 함
// 나누기 8과 나누기 4는 >> 3과 >> 2로 대체
//#define RGB( r, g, b )      ( ( ( BYTE )( r ) >> 3 ) << 11 | \
  //              ( ( ( BYTE )( g ) >> 2 ) ) << 5 |  ( ( BYTE )( b ) >> 3 ) )

//16비트로 전환하는 방법
//16비트 부분의 RGB 매크로를 사용하고
//kMemSetDWord 함수를 kMemSetWord 함수로 교환할 것

// 사각형의 정보를 담는 자료구조
typedef struct kRectangleStruct
{
	int left;
	int top;
	int right;
	int bottom;
} RECT;


typedef struct tag_RectangleInfo
{
	int left;
	int top;
	int width;
	int height;
} RECTINFO;


// 점의 정보를 담는 자료구조
typedef struct kPointStruct
{
	// X와 Y의 좌표
	int iX;
	int iY;
} POINT;

#endif

typedef struct kTEXTCOLOR
{
	COLOR textColor;
	COLOR backgroundColor;
} TEXTCOLOR;

// 마우스 이벤트 자료구조
typedef struct kMouseEventStruct
{
	// 윈도우 ID
	QWORD qwWindowID;

	// 마우스 X,Y좌표와 버튼의 상태
	POINT stPoint;
	BYTE bButtonStatus;
} MOUSEEVENT;

// 키 이벤트 자료구조
typedef struct kKeyEventStruct
{
	// 윈도우 ID
	QWORD qwWindowID;

	// 키의 ASCII 코드와 스캔 코드
	BYTE bASCIICode;
	BYTE bScanCode;

	// 키 플래그
	BYTE bFlags;
} KEYEVENT;

// 윈도우 이벤트 자료구조
typedef struct kWindowEventStruct
{
	// 윈도우 ID
	QWORD qwWindowID;

	// 영역 정보
	RECT stArea;
	//문자열 출력 이벤트 관련 데이터
	char stringEvent[4096];
} WINDOWEVENT;

// 이벤트 자료구조
typedef struct kEventStruct
{
	// 이벤트 타입
	QWORD qwType;

	// 이벤트 데이터 영역을 정의한 공용체
	union
	{
		// 마우스 이벤트 관련 데이터
		MOUSEEVENT stMouseEvent;

		// 키 이벤트 관련 데이터
		KEYEVENT stKeyEvent;

		// 윈도우 이벤트 관련 데이터
		WINDOWEVENT stWindowEvent;

		// 위의 이벤트 외에 유저 이벤트를 위한 데이터
		DWORD vqwData[3];
	};
} EVENT;

#pragma pack(pop)