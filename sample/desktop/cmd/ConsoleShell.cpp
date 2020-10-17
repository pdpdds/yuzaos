#include "ConsoleShell.h"
#include "Keyboard.h"
#include "string.h"
#include "memory.h"
#include <stdio.h>
//#include "window.h"
#include "GUIConsole.h"
#include <include/systemcall_impl.h>
#include <ConsoleManager.h>

extern SHELLCOMMANDENTRY gs_vstCommandTable;


extern "C" void printf(const char* str, ...);
DWORD WINAPI StartConsoleShell(LPVOID parameter)
{
	GUIConsole* pConsole = (GUIConsole*)parameter;
    char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;
    CONSOLEMANAGER* pstConsoleManager;	
    
    // 콘솔을 관리하는 자료구조를 반환
    pstConsoleManager = pConsole->GetConsoleManager();
    
    // 프롬프트 출력
   
    pConsole->PrintPrompt();
	//pConsole->Printf( CONSOLESHELL_PROMPTMESSAGE );
    
    // 콘솔 셸 종료 플래그가 TRUE가 될 때까지 반복
    while( pstConsoleManager->bExit == FALSE )
    {
		QWORD topWindowId = 0;
		Syscall_GetTopWindowID(&topWindowId);
		if (pConsole->GetWindowId() != topWindowId)
		{
			Syscall_Sleep(1);
			continue;
		}

        bKey = pConsole->GetCh();

        // 콘솔 셸 종료 플래그가 설정된 경우 루프를 종료
        if( pstConsoleManager->bExit == TRUE )
        {
            break;
        }
        
        if( bKey == KEY_BACKSPACE )
        {
            if( iCommandBufferIndex > 0 )
            {
                // 현재 커서 위치를 얻어서 한 문자 앞으로 이동한 다음 공백을 출력하고 
                // 커맨드 버퍼에서 마지막 문자 삭제
				pConsole->GetCursor( &iCursorX, &iCursorY );
				pConsole->PrintStringXY( iCursorX - 1, iCursorY, " " );
				pConsole->SetCursor( iCursorX - 1, iCursorY );
                iCommandBufferIndex--;
            }
        }
        else if( bKey == KEY_ENTER || bKey == 0x0d)
        {
			pConsole->Printf( "\n" );
            
            if( iCommandBufferIndex > 0 )
            {
                // 커맨드 버퍼에 있는 명령을 실행
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
				pConsole->ExecuteCommand( vcCommandBuffer );
            }
            else
            {
                pConsole->PrintPrompt();
            }
            
  
			//pConsole->Printf( "%s", CONSOLESHELL_PROMPTMESSAGE );
            memset( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
        }
        // 시프트 키, CAPS Lock, NUM Lock, Scroll Lock은 무시
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
            ;
        }
        else if( bKey < 128 )
        {
            // TAB은 공백으로 전환
            if( bKey == KEY_TAB )
            {
                bKey = ' ';
            }
            
            // 버퍼가 남아있을 때만 가능
            if( iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
            {
                vcCommandBuffer[ iCommandBufferIndex++ ] = (char)bKey;
				pConsole->Printf( "%c", (char)bKey );
            }
        }

		Syscall_Sleep(1);
    }

	return 0;
}

/**
 *  파라미터 자료구조를 초기화
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = strlen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  공백으로 구분된 파라미터의 내용과 길이를 반환
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // 더 이상 파라미터가 없으면 나감
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // 버퍼의 길이만큼 이동하면서 공백을 검색
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // 파라미터를 복사하고 길이를 반환
    memcpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // 파라미터의 위치 업데이트
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  커맨드를 처리하는 코드
//==============================================================================
/**
 *  셸 도움말을 출력
 */
void kHelp(GUIConsole* pConsole, const char* pcCommandBuffer )
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
	pConsole->Printf( "=========================================================\n" );
	pConsole->Printf( "                    SKYOS32 Shell Help                    \n" );
	pConsole->Printf( "=========================================================\n" );
    
	iCount = pConsole->GetCommandCount();
	SHELLCOMMANDENTRY* vstCommandTable = pConsole->GetCommandArray();

    // 가장 긴 커맨드의 길이를 계산
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = strlen(vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // 도움말 출력
    for( i = 0 ; i < iCount ; i++ )
    {
		pConsole->Printf( "%s", vstCommandTable[ i ].pcCommand );
		pConsole->GetCursor( &iCursorX, &iCursorY );
      //  kSetCursor( iMaxCommandLength, iCursorY );
		pConsole->Printf( "  - %s\n", vstCommandTable[ i ].pcHelp );

        // 목록이 많을 경우 나눠서 보여줌
        if( ( i != 0 ) && ( ( i % 20 ) == 0 ) )
        {
			pConsole->Printf( "Press any key to continue... ('q' is exit) : " );
            if(pConsole->GetCh() == 'q' )
            {
				pConsole->Printf( "\n" );
                break;
            }        
			pConsole->Printf( "\n" );
        }
    }
}

/**
 *  화면을 지움 
 */
void kCls(GUIConsole* pConsole, const char* pcParameterBuffer )
{
    // 맨 윗줄은 디버깅 용으로 사용하므로 화면을 지운 후, 라인 1로 커서 이동
	pConsole->ClearScreen();
	pConsole->SetCursor( 0, 0 );
}

/**
 *  총 메모리 크기를 출력
 */
void kShowTotalRAMSize(GUIConsole* pConsole, const char* pcParameterBuffer )
{
	//pConsole->Printf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  PC를 재시작(Reboot)
 */
void kShutdown(GUIConsole* pConsole, const char* pcParamegerBuffer )
{
	pConsole->Printf( "System Shutdown Start...\n" );
}