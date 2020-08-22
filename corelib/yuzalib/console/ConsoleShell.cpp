#include "ConsoleShell.h"
#include "../libcrt/input/Keyboard.h"
#include "string.h"
#include "memory.h"
#include <stdio.h>
//#include "window.h"
#include "GUIConsole.h"
#include <systemcall_impl.h>
#include "ConsoleManager.h"

extern SHELLCOMMANDENTRY gs_vstCommandTable;

extern GUIConsole* pConsole;

int StartConsoleShell(int argc, char** argv)
{
    pConsole->Start();

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