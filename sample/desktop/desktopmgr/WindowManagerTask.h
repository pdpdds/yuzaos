#pragma once
#include "windef.h"
#include "2DGraphics.h"
#include "I_GUIEngine.h"

// 윈도우 매니저 태스크가 처리할 데이터나 이벤트를 통합하는 최대 개수
#define WINDOWMANAGER_DATAACCUMULATECOUNT    20
// 윈도우 크기 변경 표식의 크기
#define WINDOWMANAGER_RESIZEMARKERSIZE       20
// 윈도우 크기 변경 표식의 색깔
#define WINDOWMANAGER_COLOR_RESIZEMARKER    RGB( 210, 20, 20 )
// 윈도우 크기 변경 표식의 두께
#define WINDOWMANAGER_THICK_RESIZEMARKER    4

void kStartWindowManager(LinearBufferInfo* bufferInfo);
bool kProcessMouseData( void );
bool kProcessKeyData( void );
bool kProcessEventQueueData( void );
void kUpdate(bool isDirectBuffer);

void kDrawResizeMarker( const RECT* pstArea, bool bShowMarker );