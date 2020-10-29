/**
 *  file    ApplicationPanelTask.c
 *  date    2009/11/03
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui 
 *  brief   애플리케이션 패널에 관련된 소스 파일
 */

#include "panel.h"
#include "string.h"
#include <stdio.h>
#include <systemcall_impl.h>
#include <gdi32.h>
#include <time.h>
#include <libconfig.h>

// 애플리케이션 테이블
APPLICATIONENTRY** g_applicationTable;

// 애플리케이션 테이블에 정의된 이름 중에서 가장 긴 것
int g_iMaxNameLength = 0;

//등록된 앱의 수
int g_appCount = 0;

static void DrawApplicationListItem(int iIndex, bool bMouseOver);
static void DrawDigitalClock(QWORD qwWindowID);
static int GetMouseOverItemIndex(int iMouseY);

// 애플리케이션 패널에서 사용하는 자료구조
APPLICATIONPANELDATA gs_stApplicationPanelData;

/**
 *  애플리케이션 패널 윈도우를 생성
 */
bool CreateApplicationPanelWindow( void )
{
   QWORD qwWindowID;
   int iMouseX, iMouseY;
	RECT stScreenArea;
	//--------------------------------------------------------------------------
	// 윈도우를 생성
	//--------------------------------------------------------------------------
	// 마우스의 현재 위치를 반환
	Syscall_GetCursorPosition(&iMouseX, &iMouseY);
	Syscall_GetScreenArea(&stScreenArea);
	stScreenArea.top = stScreenArea.bottom - APPLICATIONPANEL_HEIGHT;
	Syscall_CreateWindow(&stScreenArea, APPLICATIONPANEL_TITLE, 0, &qwWindowID);
	// 윈도우를 생성하지 못했으면 실패
	if (qwWindowID == WINDOW_INVALIDID)
	{
		return FALSE;
	}

	gs_stApplicationPanelData.qwApplicationPanelID = qwWindowID;

	// 애플리케이션 패널 윈도우의 테두리와 내부를 표시
	RECT rect = stScreenArea;
	rect.bottom -= 1;
	Syscall_DrawRect(&qwWindowID, &rect, APPLICATIONPANEL_COLOR_OUTERLINE, FALSE);

	rect = stScreenArea;
	rect.left = 1;
	rect.top = 1;
	rect.right -= 1;
	rect.bottom = rect.top + APPLICATIONPANEL_HEIGHT - 2;
	Syscall_DrawRect(&qwWindowID, &rect, APPLICATIONPANEL_COLOR_MIDDLELINE, FALSE);

	rect = stScreenArea;
	rect.left = 2;
	rect.top = 2;
	rect.right -= 3;
	rect.bottom = rect.top + APPLICATIONPANEL_HEIGHT - 3;
	Syscall_DrawRect(&qwWindowID, &rect, APPLICATIONPANEL_COLOR_INNERLINE, FALSE);

	rect = stScreenArea;
	rect.left = 3;
	rect.top = 3;
	rect.right -= 4;
	rect.bottom = rect.top + APPLICATIONPANEL_HEIGHT - 6;
	Syscall_DrawRect(&qwWindowID, &rect, APPLICATIONPANEL_COLOR_BACKGROUND, TRUE);

	// 애플리케이션 패널의 왼쪽에 GUI 태스크의 리스트를 보여주는 버튼을 표시
	SetRectangleData(5, 5, 120, 25, &(gs_stApplicationPanelData.stButtonArea));
	Syscall_DrawButton(&qwWindowID, &(gs_stApplicationPanelData.stButtonArea), APPLICATIONPANEL_COLOR_ACTIVE, "Application", RGB(255, 255, 255));

	// 애플리케이션 패널 윈도우의 오른쪽에 시계를 표시
	DrawDigitalClock(qwWindowID);

	// 애플리케이션 패널을 화면에 표시
	Syscall_ShowWindow(&qwWindowID, TRUE);

    return TRUE;
}

/**
 *  애플리케이션 패널에 시계를 표시
 */
void DrawDigitalClock( QWORD qwWindowID )
{
    RECT stWindowArea;
    RECT stUpdateArea;
	static BYTE s_bPreviousHour, s_bPreviousMinute, s_bPreviousSecond;
    char vcBuffer[ 10 ] = "00:00 AM";

	struct tm timeValue;
	time_t Converted = 0;

	if (Syscall_GetSystemTime(&timeValue) != 0)
	{
		return;
	}
    
    // 이전 시간과 변화가 없으면 시계를 표시할 필요 없음
    if( ( s_bPreviousHour == timeValue.tm_hour) && ( s_bPreviousMinute == timeValue.tm_min) &&
        ( s_bPreviousSecond == timeValue.tm_sec) )
    {
        return ;
    }
    
    // 다음 비교를 위해 시, 분, 초를 업데이트
    s_bPreviousHour = timeValue.tm_hour;
    s_bPreviousMinute = timeValue.tm_min;
    s_bPreviousSecond = timeValue.tm_sec;

    // 시간이 12시가 넘으면 PM으로 변경
    if(timeValue.tm_hour >= 12 )
    {
        if(timeValue.tm_hour > 12 )
        {
			timeValue.tm_hour -= 12;
        }
        vcBuffer[ 6 ] = 'P';
    }
    
    // 시간 설정
    vcBuffer[ 0 ] = '0' + timeValue.tm_hour / 10;
    vcBuffer[ 1 ] = '0' + timeValue.tm_hour % 10;
    // 분 설정
    vcBuffer[ 3 ] = '0' + timeValue.tm_min / 10;
    vcBuffer[ 4 ] = '0' + timeValue.tm_min % 10;
    
    // 초에 따라서 가운데 :를 깜빡임
    if( (timeValue.tm_sec % 2 ) == 1 )
    {
        vcBuffer[ 2 ] = ' ';
    }
    else
    {
        vcBuffer[ 2 ] = ':';
    }
    
    // 애플리케이션 패널 윈도우의 위치를 반환
	Syscall_GetWindowArea(&qwWindowID, &stWindowArea );
    
    // 시계 영역의 테두리를 표시
    SetRectangleData( stWindowArea.right - APPLICATIONPANEL_CLOCKWIDTH - 13, 5,
                       stWindowArea.right - 5, 25, &stUpdateArea );


	Syscall_DrawRect(&qwWindowID, &stUpdateArea, APPLICATIONPANEL_COLOR_INNERLINE, FALSE );
    
    // 시계를 표시
	POINT point;
	point.iX = stUpdateArea.left + 4;
	point.iY = stUpdateArea.top + 3;
	TEXTCOLOR textColor = { RGB(255, 255, 255), APPLICATIONPANEL_COLOR_BACKGROUND };
	Syscall_DrawText(&qwWindowID , &point, &textColor, vcBuffer, strlen( vcBuffer ) );
    
    // 시계가 그려진 영역만 화면에 업데이트
	Syscall_UpdateScreenByWindowArea(&qwWindowID, &stUpdateArea );
}

/**
 *  애플리케이션 패널에 수신된 이벤트를 처리
 */
bool ProcessApplicationPanelWindowEvent( void )
{
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    bool bProcessResult;
    QWORD qwApplicationPanelID;
    QWORD qwApplicationListID;

    // 윈도우 ID 저장
    qwApplicationPanelID = gs_stApplicationPanelData.qwApplicationPanelID;
    qwApplicationListID = gs_stApplicationPanelData.qwApplicationListID;
    bProcessResult = FALSE;
    
    // 이벤트를 처리하는 루프
    while( 1 )
    {
        // 애플리케이션 패널 윈도우의 오른쪽에 시계를 표시
        DrawDigitalClock( gs_stApplicationPanelData.qwApplicationPanelID );
        
        // 이벤트 큐에서 이벤트를 수신
        if(Syscall_ReceiveEventFromWindowQueue(&qwApplicationPanelID, &stReceivedEvent )
                == FALSE )
        {
            break;
        }

        bProcessResult = TRUE;
        
        // 수신된 이벤트를 타입에 따라 나누어 처리
        switch( stReceivedEvent.qwType )
        {
            // 마우스 왼쪽 버튼 처리
        case EVENT_MOUSE_LBUTTONDOWN:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
            // 마우스 왼쪽 버튼이 애플리케이션 패널의 버튼 내부에서 눌렸으면
            // 애플리케이션 리스트 윈도우를 표시
            if( IsInRectangle( &( gs_stApplicationPanelData.stButtonArea ), 
                    pstMouseEvent->stPoint.iX, pstMouseEvent->stPoint.iY ) == FALSE )
            {
                break;
            }
            
            // 버튼이 떨어진 상태에서 눌리는 경우
            if( gs_stApplicationPanelData.bApplicationWindowVisible == FALSE )
            {
                // 버튼을 눌린 상태로 표시
				Syscall_DrawButton(&qwApplicationPanelID, &( gs_stApplicationPanelData.stButtonArea ),
                             APPLICATIONPANEL_COLOR_BACKGROUND, "Application", 
                             RGB( 255, 255, 255 ) );
                // 버튼이 있는 영역만 화면 업데이트
				Syscall_UpdateScreenByWindowArea(&qwApplicationPanelID,
                        &( gs_stApplicationPanelData.stButtonArea ) );
        
                // 애플리케이션 리스트 윈도우에 아무것도 선택되지 않은 것으로 초기화하고 
                // 윈도우를 화면에 최상위로 표시
                if( gs_stApplicationPanelData.iPreviousMouseOverIndex != -1 )
                {
                    DrawApplicationListItem( 
                        gs_stApplicationPanelData.iPreviousMouseOverIndex, FALSE );
                    gs_stApplicationPanelData.iPreviousMouseOverIndex = -1;
                }
				Syscall_MoveWindowToTop(&gs_stApplicationPanelData.qwApplicationListID );
				Syscall_ShowWindow(&gs_stApplicationPanelData.qwApplicationListID, TRUE );
                // 플래그는 화면에 표시된 것으로 설정
                gs_stApplicationPanelData.bApplicationWindowVisible = TRUE;
            }
            // 버튼이 눌린 상태에서 떨어진 경우
            else
            {
                // 애플리케이션 패널의 버튼을 떨어진 상태로 표시
				Syscall_DrawButton(&qwApplicationPanelID, &( gs_stApplicationPanelData.stButtonArea ), APPLICATIONPANEL_COLOR_ACTIVE, "Application", RGB( 255, 255, 255 ) );
                // 버튼이 있는 영역만 화면 업데이트
				Syscall_UpdateScreenByWindowArea(&qwApplicationPanelID,
                         &( gs_stApplicationPanelData.stButtonArea ) );

                // 애플리케이션 리스트 윈도우를 숨김
				Syscall_ShowWindow(&qwApplicationListID, FALSE );
                // 플래그는 화면에 표시되지 않은 것으로 설정
                gs_stApplicationPanelData.bApplicationWindowVisible = FALSE;            
            }
            break;
            
            // 그 외 이벤트 처리
        default:
            break;
        }
    }
    
    return bProcessResult;
}


bool AddApp(config_t& cfg, char* element)
{
    config_setting_t* setting;
   
    setting = config_lookup(&cfg, element);
    if (setting != NULL)
    {
        g_appCount = config_setting_length(setting);

        if(g_appCount > 0)
            g_applicationTable = (APPLICATIONENTRY**)calloc(sizeof(APPLICATIONENTRY*) * g_appCount, 1) ;
        
        int appCount = 0;

        for (int appIndex = 0; appIndex < g_appCount; ++appIndex)
        {
            config_setting_t* env = config_setting_get_elem(setting, appIndex);

            const char* name = 0;
            const char* desc = 0;

            if (!(config_setting_lookup_string(env, "name", &name)
                && config_setting_lookup_string(env, "desc", &desc)))
                continue;

            g_applicationTable[appCount] = (APPLICATIONENTRY*)calloc(sizeof(APPLICATIONENTRY), 1);

            strcpy(g_applicationTable[appCount]->appName, name);
            strcpy(g_applicationTable[appCount]->appDesc, desc);

            int iNameLength = strlen(g_applicationTable[appCount]->appName);
            if (g_iMaxNameLength < iNameLength)
            {
                g_iMaxNameLength = iNameLength;
            }

            appCount++;
        }
    }

    return true;
}

bool LoadAppList()
{
    config_t cfg;
    config_init(&cfg);
    char* config_file = "panel.cfg";

    if (!config_read_file(&cfg, config_file))
    {
        printf("panel config file load fail : %s", config_file);
        printf("%s:%d - %s\n", config_file, config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }

    AddApp(cfg, "appinfo.APPLIST");

    config_destroy(&cfg);


    return true;
}


/**
 *  애플리케이션 리스트 윈도우를 생성
 */
bool CreateApplicationListWindow( void )
{
    int i;
    QWORD qwWindowID;
    int iX;
    int iY;
    int iWindowWidth;

    LoadAppList();
    
    // 윈도우의 너비 계산, 20은 좌우 10픽셀의 여유공간
    iWindowWidth = g_iMaxNameLength * FONT_ENGLISHWIDTH + 20;
    
    // 윈도우의 위치는 애플리케이션 패널의 버튼 아래로 설정
    iX = gs_stApplicationPanelData.stButtonArea.left;
    iY = gs_stApplicationPanelData.stButtonArea.bottom + 5;
    
    // 아이템의 개수와 최대 길이로 애플리케이션 리스트 윈도우를 생성
    // 애플리케이션 윈도우는 윈도우 제목 표시줄이 필요 없으므로 속성은 NULL로 전달
	RECT rect;
	rect.left = iX;
	rect.top = 768 - g_appCount * APPLICATIONPANEL_LISTITEMHEIGHT - APPLICATIONPANEL_LISTITEMHEIGHT - APPLICATIONPANEL_LISTITEMHEIGHT;
	rect.right = rect.left + iWindowWidth;
	rect.bottom = rect.top + g_appCount * APPLICATIONPANEL_LISTITEMHEIGHT + 1;
    Syscall_CreateWindow( &rect, APPLICATIONPANEL_LISTTITLE, NULL, &qwWindowID);
    
	 // 윈도우를 생성하지 못했으면 실패
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return FALSE;
    }
    
    // 애플리케이션 패널 자료구조에 윈도우의 너비를 저장
    gs_stApplicationPanelData.iApplicationListWidth = iWindowWidth;
    
    // 시작할 때 애플리케이션 리스트는 숨겨놓음
    gs_stApplicationPanelData.bApplicationWindowVisible = FALSE;

    // 애플리케이션 패널 자료구조에 윈도우 ID를 저장하고 이전에 마우스가 위치한 아이템은 
    // 없는 것으로 설정
    gs_stApplicationPanelData.qwApplicationListID = qwWindowID;
    gs_stApplicationPanelData.iPreviousMouseOverIndex = -1;

    // 윈도우 내부에 응용프로그램 이름과 영역을 표시
    for( i = 0 ; i < g_appCount ; i++ )
    {
        DrawApplicationListItem( i, FALSE );
    }
    
	Syscall_MoveWindow(&qwWindowID, rect.left, rect.top + 5);
    return TRUE;
}

/**
 *  애플리케이션 리스트 윈도우에 GUI 태스크 아이템을 표시
 */
void DrawApplicationListItem( int iIndex, bool bMouseOver )
{
    QWORD qwWindowID;
    int iWindowWidth;
    COLOR stColor;
    RECT stItemArea;
    
    // 애플리케이션 리스트 윈도우의 ID와 너비
    qwWindowID = gs_stApplicationPanelData.qwApplicationListID;
    iWindowWidth = gs_stApplicationPanelData.iApplicationListWidth;
    
    // 마우스가 위에 있는지 여부에 따라 내부 색을 다르게 표시
    if( bMouseOver == TRUE )
    {
        stColor = APPLICATIONPANEL_COLOR_ACTIVE;
    }
    else
    {
        stColor = APPLICATIONPANEL_COLOR_BACKGROUND;        
    }
    
    // 리스트 아이템에 테두리를 표시
	SetRectangleData(0, iIndex * APPLICATIONPANEL_LISTITEMHEIGHT,
		iWindowWidth - 1, (iIndex + 1) * APPLICATIONPANEL_LISTITEMHEIGHT,
        &stItemArea );
	Syscall_DrawRect(&qwWindowID, &stItemArea, APPLICATIONPANEL_COLOR_INNERLINE, FALSE );
    
    // 리스트 아이템의 내부를 채움
	RECT rect = stItemArea;
	rect.left += 1;
	rect.top += 1;
	rect.right -= 1;
	rect.bottom -= 1;
	Syscall_DrawRect(&qwWindowID, &rect, stColor, TRUE );
    
	POINT loc;
	loc.iX = stItemArea.left + 10;
	loc.iY = stItemArea.top + 2;
    // GUI 태스크의 이름을 표시
	TEXTCOLOR textColor = { RGB(255, 255, 255), stColor };
	Syscall_DrawText(&qwWindowID, &loc, &textColor, g_applicationTable[iIndex]->appName, strlen(g_applicationTable[iIndex]->appName) );
    
    // 업데이트된 아이템을 화면에 업데이트
	Syscall_UpdateScreenByWindowArea(&qwWindowID, &stItemArea );
}

/**
 *  애플리케이션 리스트에 수신된 이벤트를 처리
 */
bool ProcessApplicationListWindowEvent( void )
{
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
	bool bProcessResult;
    QWORD qwApplicationPanelID;
    QWORD qwApplicationListID;
    int iMouseOverIndex;
    EVENT stEvent;
    
    // 윈도우 ID 저장
    qwApplicationPanelID = gs_stApplicationPanelData.qwApplicationPanelID;
    qwApplicationListID = gs_stApplicationPanelData.qwApplicationListID;
    bProcessResult = FALSE;
    
    // 이벤트를 처리하는 루프
    while( 1 )
    {
        // 이벤트 큐에서 이벤트를 수신
        if(Syscall_ReceiveEventFromWindowQueue(&qwApplicationListID, &stReceivedEvent )
                == FALSE )
        {
            break;
        }

        bProcessResult = TRUE;
        
        // 수신된 이벤트를 타입에 따라 나누어 처리
        switch( stReceivedEvent.qwType )
        {
            // 마우스 이동 처리
        case EVENT_MOUSE_MOVE:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            // 마우스가 위치한 아이템 계산
            iMouseOverIndex = GetMouseOverItemIndex( pstMouseEvent->stPoint.iY );
            
            // 현재 마우스가 위치한 아이템과 이전에 위치한 아이템이 다를 때만 수행
            if( ( iMouseOverIndex == gs_stApplicationPanelData.iPreviousMouseOverIndex ) ||
                ( iMouseOverIndex == -1 ) )
            {
                break;
            }
            
            // 이전에 마우스가 위치한 아이템은 기본 상태로 표시
            if( gs_stApplicationPanelData.iPreviousMouseOverIndex != -1 )
            {
                DrawApplicationListItem( 
                    gs_stApplicationPanelData.iPreviousMouseOverIndex, FALSE );
            }
            
            // 지금 마우스 커서가 있는 위치는 마우스가 위치한 상태로 표시
            DrawApplicationListItem( iMouseOverIndex, TRUE );
            
            // 마우스가 위치한 아이템을 저장해둠
            gs_stApplicationPanelData.iPreviousMouseOverIndex = iMouseOverIndex;            
            break;
            
            // 마우스 왼쪽 버튼 처리
        case EVENT_MOUSE_LBUTTONDOWN:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
            
            // 지금 마우스 커서가 있는 위치는 선택된 것으로 표시
            iMouseOverIndex = GetMouseOverItemIndex( pstMouseEvent->stPoint.iY );
            if( iMouseOverIndex == -1 )
            {
                break;
            }

			Syscall_CreateProcess(g_applicationTable[iMouseOverIndex]->appName, nullptr, 16);
			

            // 애플리케이션 패널에 마우스 왼쪽 버튼이 눌렸다는 메시지를 전송하여 처리
			SetMouseEvent( qwApplicationPanelID, EVENT_MOUSE_LBUTTONDOWN,
                    gs_stApplicationPanelData.stButtonArea.left + 1,
                    gs_stApplicationPanelData.stButtonArea.top + 1,
                    NULL, &stEvent );
			Syscall_SendEventToWindow(&qwApplicationPanelID, &stEvent );
            break;
            
            // 그 외 이벤트 처리
        default:
            break;
        }
    }
    
    return bProcessResult;
}

/**
 *  마우스 커서가 위치한 애플리케이션 리스트 윈도우의 아이템 인덱스를 반환
 */
static int GetMouseOverItemIndex( int iMouseY )
{
    int iItemIndex;
    
    // 마우스 좌표로 아이템의 인덱스를 계산
    iItemIndex = iMouseY / APPLICATIONPANEL_LISTITEMHEIGHT;
    // 범위를 벗어나면 -1을 반환
    if( (g_appCount < 0 ) || ( iItemIndex >= g_appCount) )
    {
        return -1;
    }
    
    return iItemIndex;
}
