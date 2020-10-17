#include "windef.h"
#include <memory.h>
#include <skyoswindow.h>
#include <systemcall_impl.h>
#include <SkyInputHandler.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include <gdi32.h>

#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

// 한글 폰트의 너비와 길이
#define FONT_HANGULWIDTH   16
#define FONT_HANGULHEIGHT  16

/**
 *  사각형의 너비를 반환
 */
inline int GetRectangleWidth(const RECT* pstArea)
{
	int iWidth;

	iWidth = pstArea->right - pstArea->left + 1;

	if (iWidth < 0)
	{
		return -iWidth;
	}

	return iWidth;
}

/**
 *  사각형의 높이를 반환
 */
inline int GetRectangleHeight(const RECT* pstArea)
{
	int iHeight;

	iHeight = pstArea->bottom - pstArea->top + 1;

	if (iHeight < 0)
	{
		return -iHeight;
	}

	return iHeight;
}


#define MAXPROCESSORCOUNT 1
DWORD kGetProcessorLoad(int index)
{
	return 0;
}

DWORD GetTotalRAMSize()
{
	return 0;
}

DWORD kGetTaskCount(int index)
{
	return 0;
}

void GetDynamicMemoryInformation(QWORD* pqwDynamicMemoryStartAddress,
	QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize,
	QWORD* pqwUsedMemorySize)
{
	*pqwDynamicMemoryTotalSize = 0;
	*pqwUsedMemorySize = 0;
	*pqwMetaDataSize = 0;
	*pqwDynamicMemoryStartAddress = 0;
}

static void DrawProcessorInformation(QWORD qwWindowID, int iX, int iY, BYTE bAPICID);
static void DrawMemoryInformation(QWORD qwWindowID, int iY, int iWindowWidth);

int main(int argc, char** argv)
{
	QWORD qwWindowID;
	int i;
	int iWindowWidth;
	int iProcessorCount;
	DWORD vdwLastCPULoad[MAXPROCESSORCOUNT];
	int viLastTaskCount[MAXPROCESSORCOUNT];
	QWORD qwLastTickCount;
	EVENT stReceivedEvent;
	WINDOWEVENT* pstWindowEvent;
	bool bChanged;
	RECT stScreenArea;
	QWORD qwLastDynamicMemoryUsedSize;
	QWORD qwDynamicMemoryUsedSize;
	QWORD qwTemp;

	//--------------------------------------------------------------------------
	// 윈도우를 생성
	//--------------------------------------------------------------------------
	// 화면 영역의 크기를 반환
	Syscall_GetScreenArea(&stScreenArea);

	// 현재 프로세서의 개수로 윈도우의 너비를 계산
	//iProcessorCount = kGetProcessorCount();
	iProcessorCount = 1;
	iWindowWidth = iProcessorCount * (SYSTEMMONITOR_PROCESSOR_WIDTH +
		SYSTEMMONITOR_PROCESSOR_MARGIN) + SYSTEMMONITOR_PROCESSOR_MARGIN;

	// 윈도우를 화면 가운데에 생성한 뒤 화면에 표시하지 않음. 프로세서 정보와 메모리 정보를
	// 표시하는 영역을 그린 뒤 화면에 표시
	RECT rect;
	rect.left = (stScreenArea.right - iWindowWidth) / 2;
	rect.top = (stScreenArea.bottom - SYSTEMMONITOR_WINDOW_HEIGHT) / 2;
	rect.right = rect.left + iWindowWidth; 
	rect.bottom = rect.top + SYSTEMMONITOR_WINDOW_HEIGHT;;
	Syscall_CreateWindow(&rect, "System Monitor", WINDOW_FLAGS_DEFAULT & ~WINDOW_FLAGS_SHOW, &qwWindowID);
	
	// 윈도우를 생성하지 못했으면 실패
	if( qwWindowID == WINDOW_INVALIDID )
	{
		return 0;
	}

	// 프로세서 정보를 표시하는 영역을 3픽셀 두께로 표시하고 문자열을 출력
	Syscall_DrawLine(5, WINDOW_TITLEBAR_HEIGHT + 15, iWindowWidth - 5,
			WINDOW_TITLEBAR_HEIGHT + 15, RGB( 0, 0, 0 ) );
	Syscall_DrawLine( 5, WINDOW_TITLEBAR_HEIGHT + 16, iWindowWidth - 5,
			WINDOW_TITLEBAR_HEIGHT + 16, RGB( 0, 0, 0 ) );
	Syscall_DrawLine(5, WINDOW_TITLEBAR_HEIGHT + 17, iWindowWidth - 5,
			WINDOW_TITLEBAR_HEIGHT + 17, RGB( 0, 0, 0 ) );

	POINT loc{ 9, WINDOW_TITLEBAR_HEIGHT + 8 };
	TEXTCOLOR textColor{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	Syscall_DrawText(&qwWindowID, &loc, &textColor, "Processor Information", 21 );


	// 메모리 정보를 표시하는 영역을 3픽셀 두께로 표시하고 문자열을 출력
	Syscall_DrawLine( 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 50,
			iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 50,
			RGB( 0, 0, 0 ) );
	Syscall_DrawLine( 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 51,
				iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 51,
				RGB( 0, 0, 0 ) );
	Syscall_DrawLine( 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 52,
				iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 52,
				RGB( 0, 0, 0 ) );
	POINT loc2{ 9, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 43 };
	TEXTCOLOR textColor2{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	Syscall_DrawText(&qwWindowID, &loc2, &textColor2, "Memory Information", 18 );
	// 윈도우를 화면에 표시
	Syscall_ShowWindow(&qwWindowID, TRUE );

	// 루프를 돌면서 시스템 정보를 감시하여 화면에 표시
	qwLastTickCount = 0;

	// 마지막으로 측정한 프로세서의 부하와 태스크 수, 그리고 메모리 사용량은 모두 0으로 설정
	memset( vdwLastCPULoad, 0, sizeof( vdwLastCPULoad ) );
	memset( viLastTaskCount, 0, sizeof( viLastTaskCount ) );
	qwLastDynamicMemoryUsedSize = 0;

	//--------------------------------------------------------------------------
	// GUI 태스크의 이벤트 처리 루프
	//--------------------------------------------------------------------------
	while( 1 )
	{
		//----------------------------------------------------------------------
		// 이벤트 큐의 이벤트 처리
		//----------------------------------------------------------------------
		// 이벤트 큐에서 이벤트를 수신
		if( Syscall_ReceiveEventFromWindowQueue(&qwWindowID, &stReceivedEvent ) == TRUE )
		{
			// 수신된 이벤트를 타입에 따라 나누어 처리
			switch( stReceivedEvent.qwType )
			{
				// 윈도우 이벤트 처리
			case EVENT_WINDOW_CLOSE:
				//--------------------------------------------------------------
				// 윈도우 닫기 이벤트이면 윈도우를 삭제하고 루프를 빠져나가 태스크를 종료
				//--------------------------------------------------------------
				// 윈도우 삭제
				Syscall_DeleteWindow(&qwWindowID );
				return 0;
				break;

				// 그 외 정보
			default:
				break;
			}
		}

		// 0.5초마다 한번씩 시스템 상태를 확인
		if( ( Syscall_GetTickCount() - qwLastTickCount ) < 500 )
		{
			Syscall_Sleep( 1 );
			continue;
		}

		// 마지막으로 측정한 시간을 최신으로 업데이트
		qwLastTickCount = Syscall_GetTickCount();

		//----------------------------------------------------------------------
		// 프로세서 정보 출력
		//----------------------------------------------------------------------
		// 프로세서 수만큼 부하와 태스크 수를 확인하여 달라진 점이 있으면 화면에 업데이트
		for( i = 0 ; i < iProcessorCount ; i++ )
		{
			bChanged = FALSE;

			// 프로세서 부하 검사
			if( vdwLastCPULoad[ i ] != kGetProcessorLoad( i ) )
			{
				vdwLastCPULoad [ i ] = kGetProcessorLoad( i );
				bChanged = TRUE;
			}
			// 태스크 수 검사
			else if( viLastTaskCount[ i ] != kGetTaskCount( i ) )
			{
				viLastTaskCount[ i ] = kGetTaskCount( i );
				bChanged = TRUE;
			}

			// 이전과 비교해서 변경 사항이 있으면 화면에 업데이트
			if( bChanged == TRUE )
			{
				// 화면에 현재 프로세서의 부하를 표시
				DrawProcessorInformation( qwWindowID, i * SYSTEMMONITOR_PROCESSOR_WIDTH +
					( i + 1 ) * SYSTEMMONITOR_PROCESSOR_MARGIN, WINDOW_TITLEBAR_HEIGHT + 28,
					i );
			}
		}

		//----------------------------------------------------------------------
		// 동적 메모리 정보 출력
		//----------------------------------------------------------------------
		// 동적 메모리의 정보를 반환
		GetDynamicMemoryInformation( &qwTemp, &qwTemp, &qwTemp,
				&qwDynamicMemoryUsedSize );

		// 현재 동적 할당 메모리 사용량이 이전과 다르다면 메모리 정보를 출력
		if( qwDynamicMemoryUsedSize != qwLastDynamicMemoryUsedSize )
		{
			qwLastDynamicMemoryUsedSize = qwDynamicMemoryUsedSize;

			// 메모리 정보를 출력
			DrawMemoryInformation( qwWindowID, WINDOW_TITLEBAR_HEIGHT +
					SYSTEMMONITOR_PROCESSOR_HEIGHT + 60, iWindowWidth );
		}
	}
}

/**
 *  개별 프로세서의 정보를 화면에 표시
 */
 static void DrawProcessorInformation( QWORD qwWindowID, int iX, int iY, BYTE bAPICID )
 {
	 char vcBuffer[ 100 ];
	 RECT stArea;
	 QWORD qwProcessorLoad;
	 QWORD iUsageBarHeight;
	 int iMiddleX;

	 // 프로세서 ID를 표시
	 sprintf( vcBuffer, "Processor ID: %d", bAPICID );
	 POINT loc{ iX + 10, iY };
	 TEXTCOLOR textColor{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	 Syscall_DrawText(&qwWindowID, &loc, &textColor, vcBuffer, strlen( vcBuffer ) );

	 // 프로세서의 태스크 개수를 표시
	 sprintf( vcBuffer, "Task Count: %d   ", kGetTaskCount( bAPICID ) );
	 POINT loc2{ iX + 10, iY + 18 };
	 TEXTCOLOR textColor2{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	 Syscall_DrawText(&qwWindowID, &loc2, &textColor2,
			 vcBuffer, strlen( vcBuffer ) );

	 //--------------------------------------------------------------------------
	 // 프로세서 부하를 나타내는 막대를 표시
	 //--------------------------------------------------------------------------
	 // 프로세서 부하를 표시
	 qwProcessorLoad = kGetProcessorLoad( bAPICID );
	 if( qwProcessorLoad > 100 )
	 {
		 qwProcessorLoad = 100;
	 }

	 // 부하를 표시하는 막대의 전체에 테두리를 표시
	 RECT rect{ iX, iY + 36, iX + SYSTEMMONITOR_PROCESSOR_WIDTH, iY + SYSTEMMONITOR_PROCESSOR_HEIGHT };
	 Syscall_DrawRect(&qwWindowID, &rect, RGB( 0, 0, 0 ), FALSE );

	 // 프로세서 사용량을 나타내는 막대의 길이, ( 막대 전체의 길이 * 프로세서 부하 / 100 )
	 iUsageBarHeight = ( SYSTEMMONITOR_PROCESSOR_HEIGHT - 40 ) * qwProcessorLoad / 100;

	 // 부하를 표시하는 영역의 막대 내부를 표시
	 // 채워진 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠
	 RECT rect2{ iX + 2, iY + (SYSTEMMONITOR_PROCESSOR_HEIGHT - iUsageBarHeight) - 2, iX + SYSTEMMONITOR_PROCESSOR_WIDTH - 2, iY + SYSTEMMONITOR_PROCESSOR_HEIGHT - 2 };
	 Syscall_DrawRect(&qwWindowID, &rect2, SYSTEMMONITOR_BAR_COLOR, TRUE );
	 // 빈 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠
	 RECT rect3{ iX + 2, iY + 38, iX + SYSTEMMONITOR_PROCESSOR_WIDTH - 2,
			 iY + (SYSTEMMONITOR_PROCESSOR_HEIGHT - iUsageBarHeight) - 1 };
	 Syscall_DrawRect(&qwWindowID, &rect3, WINDOW_COLOR_BACKGROUND, TRUE );

	 // 프로세서의 부하를 표시, 막대의 가운데에 부하가 표시되도록 함
	 sprintf( vcBuffer, "Usage: %d%%", qwProcessorLoad );
	 iMiddleX = ( SYSTEMMONITOR_PROCESSOR_WIDTH -
			 (strlen( vcBuffer ) * FONT_ENGLISHWIDTH ) ) / 2;

	 POINT loc3{ iX + iMiddleX, iY + 80 };
	 TEXTCOLOR textColor3{ RGB(0, 0, 0),  WINDOW_COLOR_BACKGROUND };
	 Syscall_DrawText(&qwWindowID, &loc3, &textColor3, vcBuffer, strlen( vcBuffer ) );

	 // 프로세서 정보가 표시된 영역만 다시 화면에 업데이트
	 SetRectangleData( iX, iY, iX + SYSTEMMONITOR_PROCESSOR_WIDTH,
			 iY + SYSTEMMONITOR_PROCESSOR_HEIGHT, &stArea );
	 Syscall_UpdateScreenByWindowArea(&qwWindowID, &stArea );
 }
 
 
 static void DrawMemoryInformation(QWORD qwWindowID, int iY, int iWindowWidth)
 {
	 char vcBuffer[100];
	 QWORD qwTotalRAMKbyteSize;
	 QWORD qwDynamicMemoryStartAddress;
	 QWORD qwDynamicMemoryUsedSize;
	 QWORD qwUsedPercent;
	 QWORD qwTemp;
	 int iUsageBarWidth;
	 RECT stArea;
	 int iMiddleX;

	 // Mbyte 단위의 메모리를 Kbyte 단위로 변환
	 qwTotalRAMKbyteSize = GetTotalRAMSize() * 1024;

	 // 메모리 정보를 표시
	 sprintf(vcBuffer, "Total Size: %d KB        ", qwTotalRAMKbyteSize);
	 POINT loc{ SYSTEMMONITOR_PROCESSOR_MARGIN + 10, iY + 3 };
	 TEXTCOLOR textColor{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	 Syscall_DrawText(&qwWindowID, &loc, &textColor, vcBuffer, strlen(vcBuffer));

	 // 동적 메모리의 정보를 반환
	 GetDynamicMemoryInformation(&qwDynamicMemoryStartAddress, &qwTemp,
		 &qwTemp, &qwDynamicMemoryUsedSize);

	 sprintf(vcBuffer, "Used Size: %d KB        ", (qwDynamicMemoryUsedSize +
		 qwDynamicMemoryStartAddress) / 1024);
	 POINT loc2{ SYSTEMMONITOR_PROCESSOR_MARGIN + 10, iY + 21 };
	 TEXTCOLOR textColor2{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	 Syscall_DrawText(&qwWindowID, &loc2, &textColor2, vcBuffer, strlen(vcBuffer));

	 //--------------------------------------------------------------------------
	 // 메모리 사용량을 나타내는 막대를 표시
	 //--------------------------------------------------------------------------
	 // 메모리 사용량을 표시하는 막대의 전체에 테두리를 표시
	 RECT rect{ SYSTEMMONITOR_PROCESSOR_MARGIN, iY + 40,
		 iWindowWidth - SYSTEMMONITOR_PROCESSOR_MARGIN,
		 iY + SYSTEMMONITOR_MEMORY_HEIGHT - 32 };
	 Syscall_DrawRect(&qwWindowID, &rect, RGB(0, 0, 0), FALSE);
	 // 메모리 사용량(%)을 계산
	 qwUsedPercent = (qwDynamicMemoryStartAddress + qwDynamicMemoryUsedSize) *
		 100 / 1024 / qwTotalRAMKbyteSize;
	 if (qwUsedPercent > 100)
	 {
		 qwUsedPercent = 100;
	 }

	 // 메모리 사용량을 나타내는 막대의 길이, ( 막대 전체의 길이 * 메모리 사용량 / 100 )
	 iUsageBarWidth = (iWindowWidth - 2 * SYSTEMMONITOR_PROCESSOR_MARGIN) *
		 qwUsedPercent / 100;

	 // 메모리 사용량을 표시하는 영역의 막대 내부를 표시
	 // 색칠된 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠
	 RECT rect2{ SYSTEMMONITOR_PROCESSOR_MARGIN + 2, iY + 42,
		 SYSTEMMONITOR_PROCESSOR_MARGIN + 2 + iUsageBarWidth,
		 iY + SYSTEMMONITOR_MEMORY_HEIGHT - 34 };
	 Syscall_DrawRect(&qwWindowID, &rect2, SYSTEMMONITOR_BAR_COLOR, TRUE);
	 // 빈 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠
	 RECT rect3{ SYSTEMMONITOR_PROCESSOR_MARGIN + 2 + iUsageBarWidth,
		 iY + 42, iWindowWidth - SYSTEMMONITOR_PROCESSOR_MARGIN - 2,
		 iY + SYSTEMMONITOR_MEMORY_HEIGHT - 34 };
	 Syscall_DrawRect(&qwWindowID, &rect3, WINDOW_COLOR_BACKGROUND, TRUE);

	 // 사용량을 나타내는 텍스트 표시, 막대의 가운데에 사용량이 표시되도록 함
	 sprintf(vcBuffer, "Usage: %d%%", qwUsedPercent);
	 iMiddleX = (iWindowWidth - (strlen(vcBuffer) * FONT_ENGLISHWIDTH)) / 2;

	 POINT loc3{ iMiddleX, iY + 45 };
	 TEXTCOLOR textColor3{ RGB(0, 0, 0), WINDOW_COLOR_BACKGROUND };
	 Syscall_DrawText(&qwWindowID, &loc3, &textColor3,
		 vcBuffer, strlen(vcBuffer));

	 // 메모리 정보가 표시된 영역만 화면에 다시 업데이트
	 SetRectangleData(0, iY, iWindowWidth, iY + SYSTEMMONITOR_MEMORY_HEIGHT, &stArea);
	 Syscall_UpdateScreenByWindowArea(&qwWindowID, &stArea);
 }
