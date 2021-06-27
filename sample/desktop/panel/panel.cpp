/**
 *  file    ApplicationPanelTask.c
 *  date    2009/11/03
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui 
 *  brief   ���ø����̼� �гο� ���õ� �ҽ� ����
 */

#include "panel.h"
#include "string.h"
#include <stdio.h>
#include <systemcall_impl.h>
#include <gdi32.h>
#include <time.h>
#include <libconfig.h>

// ���ø����̼� ���̺�
APPLICATIONENTRY** g_applicationTable;

// ���ø����̼� ���̺� ���ǵ� �̸� �߿��� ���� �� ��
int g_iMaxNameLength = 0;

//��ϵ� ���� ��
int g_appCount = 0;

static void DrawApplicationListItem(int iIndex, bool bMouseOver);
static void DrawDigitalClock(QWORD qwWindowID);
static int GetMouseOverItemIndex(int iMouseY);

// ���ø����̼� �гο��� ����ϴ� �ڷᱸ��
APPLICATIONPANELDATA gs_stApplicationPanelData;

/**
 *  ���ø����̼� �г� �����츦 ����
 */
bool CreateApplicationPanelWindow( void )
{
   QWORD qwWindowID;
   int iMouseX, iMouseY;
	RECT stScreenArea;
	//--------------------------------------------------------------------------
	// �����츦 ����
	//--------------------------------------------------------------------------
	// ���콺�� ���� ��ġ�� ��ȯ
	Syscall_GetCursorPosition(&iMouseX, &iMouseY);
	Syscall_GetScreenArea(&stScreenArea);
	stScreenArea.top = stScreenArea.bottom - APPLICATIONPANEL_HEIGHT;
	Syscall_CreateWindow(&stScreenArea, APPLICATIONPANEL_TITLE, 0, &qwWindowID);
	// �����츦 �������� �������� ����
	if (qwWindowID == WINDOW_INVALIDID)
	{
		return FALSE;
	}

	gs_stApplicationPanelData.qwApplicationPanelID = qwWindowID;

	// ���ø����̼� �г� �������� �׵θ��� ���θ� ǥ��
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

	// ���ø����̼� �г��� ���ʿ� GUI �½�ũ�� ����Ʈ�� �����ִ� ��ư�� ǥ��
	SetRectangleData(5, 5, 120, 25, &(gs_stApplicationPanelData.stButtonArea));
	Syscall_DrawButton(&qwWindowID, &(gs_stApplicationPanelData.stButtonArea), APPLICATIONPANEL_COLOR_ACTIVE, "Application", RGB(255, 255, 255));

	// ���ø����̼� �г� �������� �����ʿ� �ð踦 ǥ��
	DrawDigitalClock(qwWindowID);

	// ���ø����̼� �г��� ȭ�鿡 ǥ��
	Syscall_ShowWindow(&qwWindowID, TRUE);

    return TRUE;
}

/**
 *  ���ø����̼� �гο� �ð踦 ǥ��
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
    
    // ���� �ð��� ��ȭ�� ������ �ð踦 ǥ���� �ʿ� ����
    if( ( s_bPreviousHour == timeValue.tm_hour) && ( s_bPreviousMinute == timeValue.tm_min) &&
        ( s_bPreviousSecond == timeValue.tm_sec) )
    {
        return ;
    }
    
    // ���� �񱳸� ���� ��, ��, �ʸ� ������Ʈ
    s_bPreviousHour = timeValue.tm_hour;
    s_bPreviousMinute = timeValue.tm_min;
    s_bPreviousSecond = timeValue.tm_sec;

    // �ð��� 12�ð� ������ PM���� ����
    if(timeValue.tm_hour >= 12 )
    {
        if(timeValue.tm_hour > 12 )
        {
			timeValue.tm_hour -= 12;
        }
        vcBuffer[ 6 ] = 'P';
    }
    
    // �ð� ����
    vcBuffer[ 0 ] = '0' + timeValue.tm_hour / 10;
    vcBuffer[ 1 ] = '0' + timeValue.tm_hour % 10;
    // �� ����
    vcBuffer[ 3 ] = '0' + timeValue.tm_min / 10;
    vcBuffer[ 4 ] = '0' + timeValue.tm_min % 10;
    
    // �ʿ� ���� ��� :�� ������
    if( (timeValue.tm_sec % 2 ) == 1 )
    {
        vcBuffer[ 2 ] = ' ';
    }
    else
    {
        vcBuffer[ 2 ] = ':';
    }
    
    // ���ø����̼� �г� �������� ��ġ�� ��ȯ
	Syscall_GetWindowArea(&qwWindowID, &stWindowArea );
    
    // �ð� ������ �׵θ��� ǥ��
    SetRectangleData( stWindowArea.right - APPLICATIONPANEL_CLOCKWIDTH - 13, 5,
                       stWindowArea.right - 5, 25, &stUpdateArea );


	Syscall_DrawRect(&qwWindowID, &stUpdateArea, APPLICATIONPANEL_COLOR_INNERLINE, FALSE );
    
    // �ð踦 ǥ��
	POINT point;
	point.iX = stUpdateArea.left + 4;
	point.iY = stUpdateArea.top + 3;
	TEXTCOLOR textColor = { RGB(255, 255, 255), APPLICATIONPANEL_COLOR_BACKGROUND };
	Syscall_DrawText(&qwWindowID , &point, &textColor, vcBuffer, strlen( vcBuffer ) );
    
    // �ð谡 �׷��� ������ ȭ�鿡 ������Ʈ
	Syscall_UpdateScreenByWindowArea(&qwWindowID, &stUpdateArea );
}

/**
 *  ���ø����̼� �гο� ���ŵ� �̺�Ʈ�� ó��
 */
bool ProcessApplicationPanelWindowEvent( void )
{
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    bool bProcessResult;
    QWORD qwApplicationPanelID;
    QWORD qwApplicationListID;

    // ������ ID ����
    qwApplicationPanelID = gs_stApplicationPanelData.qwApplicationPanelID;
    qwApplicationListID = gs_stApplicationPanelData.qwApplicationListID;
    bProcessResult = FALSE;
    
    // �̺�Ʈ�� ó���ϴ� ����
    while( 1 )
    {
        // ���ø����̼� �г� �������� �����ʿ� �ð踦 ǥ��
        DrawDigitalClock( gs_stApplicationPanelData.qwApplicationPanelID );
        
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if(Syscall_ReceiveEventFromWindowQueue(&qwApplicationPanelID, &stReceivedEvent )
                == FALSE )
        {
            break;
        }

        bProcessResult = TRUE;
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // ���콺 ���� ��ư ó��
        case EVENT_MOUSE_LBUTTONDOWN:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
            // ���콺 ���� ��ư�� ���ø����̼� �г��� ��ư ���ο��� ��������
            // ���ø����̼� ����Ʈ �����츦 ǥ��
            if( IsInRectangle( &( gs_stApplicationPanelData.stButtonArea ), 
                    pstMouseEvent->stPoint.iX, pstMouseEvent->stPoint.iY ) == FALSE )
            {
                break;
            }
            
            // ��ư�� ������ ���¿��� ������ ���
            if( gs_stApplicationPanelData.bApplicationWindowVisible == FALSE )
            {
                // ��ư�� ���� ���·� ǥ��
				Syscall_DrawButton(&qwApplicationPanelID, &( gs_stApplicationPanelData.stButtonArea ),
                             APPLICATIONPANEL_COLOR_BACKGROUND, "Application", 
                             RGB( 255, 255, 255 ) );
                // ��ư�� �ִ� ������ ȭ�� ������Ʈ
				Syscall_UpdateScreenByWindowArea(&qwApplicationPanelID,
                        &( gs_stApplicationPanelData.stButtonArea ) );
        
                // ���ø����̼� ����Ʈ �����쿡 �ƹ��͵� ���õ��� ���� ������ �ʱ�ȭ�ϰ� 
                // �����츦 ȭ�鿡 �ֻ����� ǥ��
                if( gs_stApplicationPanelData.iPreviousMouseOverIndex != -1 )
                {
                    DrawApplicationListItem( 
                        gs_stApplicationPanelData.iPreviousMouseOverIndex, FALSE );
                    gs_stApplicationPanelData.iPreviousMouseOverIndex = -1;
                }
				Syscall_MoveWindowToTop(&gs_stApplicationPanelData.qwApplicationListID );
				Syscall_ShowWindow(&gs_stApplicationPanelData.qwApplicationListID, TRUE );
                // �÷��״� ȭ�鿡 ǥ�õ� ������ ����
                gs_stApplicationPanelData.bApplicationWindowVisible = TRUE;
            }
            // ��ư�� ���� ���¿��� ������ ���
            else
            {
                // ���ø����̼� �г��� ��ư�� ������ ���·� ǥ��
				Syscall_DrawButton(&qwApplicationPanelID, &( gs_stApplicationPanelData.stButtonArea ), APPLICATIONPANEL_COLOR_ACTIVE, "Application", RGB( 255, 255, 255 ) );
                // ��ư�� �ִ� ������ ȭ�� ������Ʈ
				Syscall_UpdateScreenByWindowArea(&qwApplicationPanelID,
                         &( gs_stApplicationPanelData.stButtonArea ) );

                // ���ø����̼� ����Ʈ �����츦 ����
				Syscall_ShowWindow(&qwApplicationListID, FALSE );
                // �÷��״� ȭ�鿡 ǥ�õ��� ���� ������ ����
                gs_stApplicationPanelData.bApplicationWindowVisible = FALSE;            
            }
            break;
            
            // �� �� �̺�Ʈ ó��
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
            const char* exePath = 0;

            if (!(config_setting_lookup_string(env, "name", &name)
                && config_setting_lookup_string(env, "desc", &desc)))
                continue;

            if (!(config_setting_lookup_string(env, "path", &exePath)))
            {
                exePath = name;
            }

            g_applicationTable[appCount] = (APPLICATIONENTRY*)calloc(sizeof(APPLICATIONENTRY), 1);

            strcpy(g_applicationTable[appCount]->appName, name);
            strcpy(g_applicationTable[appCount]->appDesc, desc);
            strcpy(g_applicationTable[appCount]->exePath, exePath);

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
 *  ���ø����̼� ����Ʈ �����츦 ����
 */
bool CreateApplicationListWindow( void )
{
    int i;
    QWORD qwWindowID;
    int iX;
    int iY;
    int iWindowWidth;

    LoadAppList();
    
    // �������� �ʺ� ���, 20�� �¿� 10�ȼ��� ��������
    iWindowWidth = g_iMaxNameLength * FONT_ENGLISHWIDTH + 20;
    
    // �������� ��ġ�� ���ø����̼� �г��� ��ư �Ʒ��� ����
    iX = gs_stApplicationPanelData.stButtonArea.left;
    iY = gs_stApplicationPanelData.stButtonArea.bottom + 5;
    
    // �������� ������ �ִ� ���̷� ���ø����̼� ����Ʈ �����츦 ����
    // ���ø����̼� ������� ������ ���� ǥ������ �ʿ� �����Ƿ� �Ӽ��� NULL�� ����
	RECT rect;
	rect.left = iX;
	rect.top = 768 - g_appCount * APPLICATIONPANEL_LISTITEMHEIGHT - APPLICATIONPANEL_LISTITEMHEIGHT - APPLICATIONPANEL_LISTITEMHEIGHT;
	rect.right = rect.left + iWindowWidth;
	rect.bottom = rect.top + g_appCount * APPLICATIONPANEL_LISTITEMHEIGHT + 1;
    Syscall_CreateWindow( &rect, APPLICATIONPANEL_LISTTITLE, NULL, &qwWindowID);
    
	 // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return FALSE;
    }
    
    // ���ø����̼� �г� �ڷᱸ���� �������� �ʺ� ����
    gs_stApplicationPanelData.iApplicationListWidth = iWindowWidth;
    
    // ������ �� ���ø����̼� ����Ʈ�� ���ܳ���
    gs_stApplicationPanelData.bApplicationWindowVisible = FALSE;

    // ���ø����̼� �г� �ڷᱸ���� ������ ID�� �����ϰ� ������ ���콺�� ��ġ�� �������� 
    // ���� ������ ����
    gs_stApplicationPanelData.qwApplicationListID = qwWindowID;
    gs_stApplicationPanelData.iPreviousMouseOverIndex = -1;

    // ������ ���ο� �������α׷� �̸��� ������ ǥ��
    for( i = 0 ; i < g_appCount ; i++ )
    {
        DrawApplicationListItem( i, FALSE );
    }
    
	Syscall_MoveWindow(&qwWindowID, rect.left, rect.top + 5);
    return TRUE;
}

/**
 *  ���ø����̼� ����Ʈ �����쿡 GUI �½�ũ �������� ǥ��
 */
void DrawApplicationListItem( int iIndex, bool bMouseOver )
{
    QWORD qwWindowID;
    int iWindowWidth;
    COLOR stColor;
    RECT stItemArea;
    
    // ���ø����̼� ����Ʈ �������� ID�� �ʺ�
    qwWindowID = gs_stApplicationPanelData.qwApplicationListID;
    iWindowWidth = gs_stApplicationPanelData.iApplicationListWidth;
    
    // ���콺�� ���� �ִ��� ���ο� ���� ���� ���� �ٸ��� ǥ��
    if( bMouseOver == TRUE )
    {
        stColor = APPLICATIONPANEL_COLOR_ACTIVE;
    }
    else
    {
        stColor = APPLICATIONPANEL_COLOR_BACKGROUND;        
    }
    
    // ����Ʈ �����ۿ� �׵θ��� ǥ��
	SetRectangleData(0, iIndex * APPLICATIONPANEL_LISTITEMHEIGHT,
		iWindowWidth - 1, (iIndex + 1) * APPLICATIONPANEL_LISTITEMHEIGHT,
        &stItemArea );
	Syscall_DrawRect(&qwWindowID, &stItemArea, APPLICATIONPANEL_COLOR_INNERLINE, FALSE );
    
    // ����Ʈ �������� ���θ� ä��
	RECT rect = stItemArea;
	rect.left += 1;
	rect.top += 1;
	rect.right -= 1;
	rect.bottom -= 1;
	Syscall_DrawRect(&qwWindowID, &rect, stColor, TRUE );
    
	POINT loc;
	loc.iX = stItemArea.left + 10;
	loc.iY = stItemArea.top + 2;
    // GUI �½�ũ�� �̸��� ǥ��
	TEXTCOLOR textColor = { RGB(255, 255, 255), stColor };
	Syscall_DrawText(&qwWindowID, &loc, &textColor, g_applicationTable[iIndex]->appName, strlen(g_applicationTable[iIndex]->appName) );
    
    // ������Ʈ�� �������� ȭ�鿡 ������Ʈ
	Syscall_UpdateScreenByWindowArea(&qwWindowID, &stItemArea );
}

/**
 *  ���ø����̼� ����Ʈ�� ���ŵ� �̺�Ʈ�� ó��
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
    
    // ������ ID ����
    qwApplicationPanelID = gs_stApplicationPanelData.qwApplicationPanelID;
    qwApplicationListID = gs_stApplicationPanelData.qwApplicationListID;
    bProcessResult = FALSE;
    
    // �̺�Ʈ�� ó���ϴ� ����
    while( 1 )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if(Syscall_ReceiveEventFromWindowQueue(&qwApplicationListID, &stReceivedEvent )
                == FALSE )
        {
            break;
        }

        bProcessResult = TRUE;
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // ���콺 �̵� ó��
        case EVENT_MOUSE_MOVE:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            // ���콺�� ��ġ�� ������ ���
            iMouseOverIndex = GetMouseOverItemIndex( pstMouseEvent->stPoint.iY );
            
            // ���� ���콺�� ��ġ�� �����۰� ������ ��ġ�� �������� �ٸ� ���� ����
            if( ( iMouseOverIndex == gs_stApplicationPanelData.iPreviousMouseOverIndex ) ||
                ( iMouseOverIndex == -1 ) )
            {
                break;
            }
            
            // ������ ���콺�� ��ġ�� �������� �⺻ ���·� ǥ��
            if( gs_stApplicationPanelData.iPreviousMouseOverIndex != -1 )
            {
                DrawApplicationListItem( 
                    gs_stApplicationPanelData.iPreviousMouseOverIndex, FALSE );
            }
            
            // ���� ���콺 Ŀ���� �ִ� ��ġ�� ���콺�� ��ġ�� ���·� ǥ��
            DrawApplicationListItem( iMouseOverIndex, TRUE );
            
            // ���콺�� ��ġ�� �������� �����ص�
            gs_stApplicationPanelData.iPreviousMouseOverIndex = iMouseOverIndex;            
            break;
            
            // ���콺 ���� ��ư ó��
        case EVENT_MOUSE_LBUTTONDOWN:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
            
            // ���� ���콺 Ŀ���� �ִ� ��ġ�� ���õ� ������ ǥ��
            iMouseOverIndex = GetMouseOverItemIndex( pstMouseEvent->stPoint.iY );
            if( iMouseOverIndex == -1 )
            {
                break;
            }

			Syscall_CreateProcess(g_applicationTable[iMouseOverIndex]->exePath, nullptr, 16);
			

            // ���ø����̼� �гο� ���콺 ���� ��ư�� ���ȴٴ� �޽����� �����Ͽ� ó��
			SetMouseEvent( qwApplicationPanelID, EVENT_MOUSE_LBUTTONDOWN,
                    gs_stApplicationPanelData.stButtonArea.left + 1,
                    gs_stApplicationPanelData.stButtonArea.top + 1,
                    NULL, &stEvent );
			Syscall_SendEventToWindow(&qwApplicationPanelID, &stEvent );
            break;
            
            // �� �� �̺�Ʈ ó��
        default:
            break;
        }
    }
    
    return bProcessResult;
}

/**
 *  ���콺 Ŀ���� ��ġ�� ���ø����̼� ����Ʈ �������� ������ �ε����� ��ȯ
 */
static int GetMouseOverItemIndex( int iMouseY )
{
    int iItemIndex;
    
    // ���콺 ��ǥ�� �������� �ε����� ���
    iItemIndex = iMouseY / APPLICATIONPANEL_LISTITEMHEIGHT;
    // ������ ����� -1�� ��ȯ
    if( (g_appCount < 0 ) || ( iItemIndex >= g_appCount) )
    {
        return -1;
    }
    
    return iItemIndex;
}
