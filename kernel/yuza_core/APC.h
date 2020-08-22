#pragma once
#include <ds/queue.h>

// 비동기 프로시저 콜
// 함수는 커널에서 종료되는 스레드를 대신하여 커널 모드에서 호출된다.
struct APC : public _QueueNode {
	/// 비동기 프로시저 콜이 발생할때 실행되는 함수
	/// 이 함수는 유저공간 또는 커널 공간에 있을 수 있다.
	void (*fCallback)(void *data);

	/// 콜백 함수로 전달되는 데이터
	void *fData;

	/// 이 함수가 커널 모드에서의 APC면 true 그렇지 않고 유저 모드면 false
	bool fIsKernel;
};
