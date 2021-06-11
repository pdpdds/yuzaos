#include "ConsoleShell.h"
#include "Keyboard.h"
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
 *  �Ķ���� �ڷᱸ���� �ʱ�ȭ
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = strlen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  �������� ���е� �Ķ������ ����� ���̸� ��ȯ
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // �� �̻� �Ķ���Ͱ� ������ ����
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // ������ ���̸�ŭ �̵��ϸ鼭 ������ �˻�
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // �Ķ���͸� �����ϰ� ���̸� ��ȯ
    memcpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // �Ķ������ ��ġ ������Ʈ
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  Ŀ�ǵ带 ó���ϴ� �ڵ�
//==============================================================================
/**
 *  �� ������ ���
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

    // ���� �� Ŀ�ǵ��� ���̸� ���
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = strlen(vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // ���� ���
    for( i = 0 ; i < iCount ; i++ )
    {
		pConsole->Printf( "%s", vstCommandTable[ i ].pcCommand );
		pConsole->GetCursor( &iCursorX, &iCursorY );
      //  kSetCursor( iMaxCommandLength, iCursorY );
		pConsole->Printf( "  - %s\n", vstCommandTable[ i ].pcHelp );

        // ����� ���� ��� ������ ������
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
 *  ȭ���� ���� 
 */
void kCls(GUIConsole* pConsole, const char* pcParameterBuffer )
{
    // �� ������ ����� ������ ����ϹǷ� ȭ���� ���� ��, ���� 1�� Ŀ�� �̵�
	pConsole->ClearScreen();
	pConsole->SetCursor( 0, 0 );
}

/**
 *  �� �޸� ũ�⸦ ���
 */
void kShowTotalRAMSize(GUIConsole* pConsole, const char* pcParameterBuffer )
{
	//pConsole->Printf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  PC�� �����(Reboot)
 */
void kShutdown(GUIConsole* pConsole, const char* pcParamegerBuffer )
{
	pConsole->Printf( "System Shutdown Start...\n" );
}