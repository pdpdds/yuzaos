/**
 *  file    Main.c
 *  date    2009/01/09
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   Ű���� ����̽� ����̹��� ���õ� �ҽ� ����
 */

#include "windef.h"
#include "Keyboard.h"
#include "InputQueue.h"
#include "Mouse.h"
#include "systemcall_impl.h"
int g_key_sema = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Ű���� ��Ʈ�ѷ� �� Ű���� ��� ���õ� �Լ��� 
//
////////////////////////////////////////////////////////////////////////////////
/**
 *  ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� ��ȯ
 */
 /*bool kIsOutputBufferFull( void )
{
    // ���� ��������(��Ʈ 0x64)���� ���� ���� ��� ���� ���� ��Ʈ(��Ʈ 0)��
    // 1�� �����Ǿ� ������ ��� ���ۿ� Ű���尡 ������ �����Ͱ� ������
   if( kInPortByte( 0x64 ) & 0x01 )
    {
        return TRUE;
    }
    return FALSE;
}*/

/**
 * �Է� ����(��Ʈ 0x60)�� ���μ����� �� �����Ͱ� �����ִ��� ���θ� ��ȯ
 */
 /*bool kIsInputBufferFull( void )
{
    // ���� ��������(��Ʈ 0x64)���� ���� ���� �Է� ���� ���� ��Ʈ(��Ʈ 1)��
    // 1�� �����Ǿ� ������ ���� Ű���尡 �����͸� �������� �ʾ���
    if( kInPortByte( 0x64 ) & 0x02 )
    {
        return TRUE;
    }
    return FALSE;
}*/


/**
 *  Ű���带 Ȱ��ȭ ��
 */
/*bool kActivateKeyboard( void )
{
   /* int i, j;
	bool bPreviousInterrupt;
	bool bResult;
    
    // ���ͷ�Ʈ �Ұ�
    bPreviousInterrupt = kSetInterruptFlag( FALSE );
    
    // ��Ʈ�� ��������(��Ʈ 0x64)�� Ű���� Ȱ��ȭ Ŀ�ǵ�(0xAE)�� �����Ͽ� Ű���� ����̽� Ȱ��ȭ
    kOutPortByte( 0x64, 0xAE );
        
    // �Է� ����(��Ʈ 0x60)�� �� ������ ��ٷȴٰ� Ű���忡 Ȱ��ȭ Ŀ�ǵ带 ����
    // 0xFFFF��ŭ ������ ������ �ð��̸� ����� Ŀ�ǵ尡 ���۵� �� ����
    // 0xFFFF ������ ������ ���Ŀ��� �Է� ����(��Ʈ 0x60)�� ���� ������ �����ϰ� ����
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // �Է� ����(��Ʈ 0x60)�� ��������� Ű���� Ŀ�ǵ� ���� ����
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }
	
    // �Է� ����(��Ʈ 0x60)�� Ű���� Ȱ��ȭ(0xF4) Ŀ�ǵ带 �����Ͽ� Ű����� ����
    kOutPortByte( 0x60, 0xF4 );
    
    // ACK�� �� ������ �����
    bResult = kWaitForACKAndPutOtherScanCode();
    
    // ���� ���ͷ�Ʈ ���� ����
    kSetInterruptFlag( bPreviousInterrupt );
    return bResult;
}*/





/**
 *  A20 ����Ʈ�� Ȱ��ȭ
 */
/*void kEnableA20Gate( void )
{
    BYTE bOutputPortData;
    int i;
    
    // ��Ʈ�� ��������(��Ʈ 0x64)�� Ű���� ��Ʈ�ѷ��� ��� ��Ʈ ���� �д� Ŀ�ǵ�(0xD0) ����
    kOutPortByte( 0x64, 0xD0 );
    
    // ��� ��Ʈ�� �����͸� ��ٷȴٰ� ����
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // ��� ����(��Ʈ 0x60)�� �������� �����͸� ���� �� ���� 
        if( kIsOutputBufferFull() == TRUE )
        {
            break;
        }
    }
    // ��� ��Ʈ(��Ʈ 0x60)�� ���ŵ� Ű���� ��Ʈ�ѷ��� ��� ��Ʈ ���� ����
    bOutputPortData = kInPortByte( 0x60 );
    
    // A20 ����Ʈ ��Ʈ ����
    bOutputPortData |= 0x01;
    
    // �Է� ����(��Ʈ 0x60)�� �����Ͱ� ��������� ��� ��Ʈ�� ���� ���� Ŀ�ǵ�� ��� ��Ʈ ������ ����
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // �Է� ����(��Ʈ 0x60)�� ������� Ŀ�ǵ� ���� ����
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }
    
    // Ŀ�ǵ� ��������(0x64)�� ��� ��Ʈ ���� Ŀ�ǵ�(0xD1)�� ����
    kOutPortByte( 0x64, 0xD1 );
    
    // �Է� ����(0x60)�� A20 ����Ʈ ��Ʈ�� 1�� ������ ���� ����
    kOutPortByte( 0x60, bOutputPortData );
}*/

/**
 *  ���μ����� ����(Reset)
 */
/*void kReboot( void )
{
    int i;
    
    // �Է� ����(��Ʈ 0x60)�� �����Ͱ� ��������� ��� ��Ʈ�� ���� ���� Ŀ�ǵ�� ��� ��Ʈ ������ ����
    for( i = 0 ; i < 0xFFFF ; i++ )
    {
        // �Է� ����(��Ʈ 0x60)�� ������� Ŀ�ǵ� ���� ����
        if( kIsInputBufferFull() == FALSE )
        {
            break;
        }
    }
    
    // Ŀ�ǵ� ��������(0x64)�� ��� ��Ʈ ���� Ŀ�ǵ�(0xD1)�� ����
    kOutPortByte( 0x64, 0xD1 );
    
    // �Է� ����(0x60)�� 0�� �����Ͽ� ���μ����� ����(Reset)��
    kOutPortByte( 0x60, 0x00 );
    
    while( 1 )
    {
        ;
    }
}*/

////////////////////////////////////////////////////////////////////////////////
//
// ��ĵ �ڵ带 ASCII �ڵ�� ��ȯ�ϴ� ��ɿ� ���õ� �Լ���
//
////////////////////////////////////////////////////////////////////////////////
// Ű���� ���¸� �����ϴ� Ű���� �Ŵ���
static KEYBOARDMANAGER gs_stKeyboardManager = { 0, };
// Ű�� �����ϴ� ť�� ���� ����
QUEUE gs_stKeyQueue;
static KEYDATA gs_vstKeyQueueBuffer[ KEY_MAXQUEUECOUNT ];

// ��ĵ �ڵ带 ASCII �ڵ�� ��ȯ�ϴ� ���̺�
static KEYMAPPINGENTRY gs_vstKeyMappingTable[ KEY_MAPPINGTABLEMAXCOUNT ] =
{
    /*  0   */  {   KEY_NONE        ,   KEY_NONE        },
    /*  1   */  {   KEY_ESC         ,   KEY_ESC         },
    /*  2   */  {   '1'             ,   '!'             },
    /*  3   */  {   '2'             ,   '@'             },
    /*  4   */  {   '3'             ,   '#'             },
    /*  5   */  {   '4'             ,   '$'             },
    /*  6   */  {   '5'             ,   '%'             },
    /*  7   */  {   '6'             ,   '^'             },
    /*  8   */  {   '7'             ,   '&'             },
    /*  9   */  {   '8'             ,   '*'             },
    /*  10  */  {   '9'             ,   '('             },
    /*  11  */  {   '0'             ,   ')'             },
    /*  12  */  {   '-'             ,   '_'             },
    /*  13  */  {   '='             ,   '+'             },
    /*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
    /*  15  */  {   KEY_TAB         ,   KEY_TAB         },
    /*  16  */  {   'q'             ,   'Q'             },
    /*  17  */  {   'w'             ,   'W'             },
    /*  18  */  {   'e'             ,   'E'             },
    /*  19  */  {   'r'             ,   'R'             },
    /*  20  */  {   't'             ,   'T'             },
    /*  21  */  {   'y'             ,   'Y'             },
    /*  22  */  {   'u'             ,   'U'             },
    /*  23  */  {   'i'             ,   'I'             },
    /*  24  */  {   'o'             ,   'O'             },
    /*  25  */  {   'p'             ,   'P'             },
    /*  26  */  {   '['             ,   '{'             },
    /*  27  */  {   ']'             ,   '}'             },
    /*  28  */  {   '\n'            ,   '\n'            },
    /*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
    /*  30  */  {   'a'             ,   'A'             },
    /*  31  */  {   's'             ,   'S'             },
    /*  32  */  {   'd'             ,   'D'             },
    /*  33  */  {   'f'             ,   'F'             },
    /*  34  */  {   'g'             ,   'G'             },
    /*  35  */  {   'h'             ,   'H'             },
    /*  36  */  {   'j'             ,   'J'             },
    /*  37  */  {   'k'             ,   'K'             },
    /*  38  */  {   'l'             ,   'L'             },
    /*  39  */  {   ';'             ,   ':'             },
    /*  40  */  {   '\''            ,   '\"'            },
    /*  41  */  {   '`'             ,   '~'             },
    /*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
    /*  43  */  {   '\\'            ,   '|'             },
    /*  44  */  {   'z'             ,   'Z'             },
    /*  45  */  {   'x'             ,   'X'             },
    /*  46  */  {   'c'             ,   'C'             },
    /*  47  */  {   'v'             ,   'V'             },
    /*  48  */  {   'b'             ,   'B'             },
    /*  49  */  {   'n'             ,   'N'             },
    /*  50  */  {   'm'             ,   'M'             },
    /*  51  */  {   ','             ,   '<'             },
    /*  52  */  {   '.'             ,   '>'             },
    /*  53  */  {   '/'             ,   '?'             },
    /*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
    /*  55  */  {   '*'             ,   '*'             },
    /*  56  */  {   KEY_LALT        ,   KEY_LALT        },
    /*  57  */  {   ' '             ,   ' '             },
    /*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
    /*  59  */  {   KEY_F1          ,   KEY_F1          },
    /*  60  */  {   KEY_F2          ,   KEY_F2          },
    /*  61  */  {   KEY_F3          ,   KEY_F3          },
    /*  62  */  {   KEY_F4          ,   KEY_F4          },
    /*  63  */  {   KEY_F5          ,   KEY_F5          },
    /*  64  */  {   KEY_F6          ,   KEY_F6          },
    /*  65  */  {   KEY_F7          ,   KEY_F7          },
    /*  66  */  {   KEY_F8          ,   KEY_F8          },
    /*  67  */  {   KEY_F9          ,   KEY_F9          },
    /*  68  */  {   KEY_F10         ,   KEY_F10         },
    /*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
    /*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },
    
    /*  71  */  {   KEY_HOME        ,   '7'             },
    /*  72  */  {   KEY_UP          ,   '8'             },
    /*  73  */  {   KEY_PAGEUP      ,   '9'             },
    /*  74  */  {   '-'             ,   '-'             },
    /*  75  */  {   KEY_LEFT        ,   '4'             },
    /*  76  */  {   KEY_CENTER      ,   '5'             },
    /*  77  */  {   KEY_RIGHT       ,   '6'             },
    /*  78  */  {   '+'             ,   '+'             },
    /*  79  */  {   KEY_END         ,   '1'             },
    /*  80  */  {   KEY_DOWN        ,   '2'             },
    /*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
    /*  82  */  {   KEY_INS         ,   '0'             },
    /*  83  */  {   KEY_DEL         ,   '.'             },
    /*  84  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  85  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  86  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  87  */  {   KEY_F11         ,   KEY_F11         },
    /*  88  */  {   KEY_F12         ,   KEY_F12         }   
};









/**
 *  Ű���� �ʱ�ȭ
 */
bool kInitializeKeyboard( void )
{
    // ť �ʱ�ȭ
    kInitializeQueue( &gs_stKeyQueue, gs_vstKeyQueueBuffer, KEY_MAXQUEUECOUNT, 
            sizeof( KEYDATA ) );
    
	g_key_sema = Syscall_CreateSemaphore("KeyQueue", 1);

	return true;
    
}



/**
 *  Ű ť���� �����͸� ����
 */
bool kGetKeyFromKeyQueue( KEYDATA* pstData )
{
    bool bResult;
    
	Syscall_AquireSemaphore(g_key_sema, -1);
	bResult = kGetQueue(&gs_stKeyQueue, pstData);
	Syscall_ReleaseSemaphore(g_key_sema, 1);

    return bResult;
}
