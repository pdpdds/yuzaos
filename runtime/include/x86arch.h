#pragma once
#include "minwindef.h"

void	LoadPDBR(UINT32 physicalAddr);
UINT32	GetPDBR();

void	EnablePaging(bool state);
bool	IsPaging();

//캐쉬된 TLB를 비운다.
void FlushTranslationLockBufferEntry(UINT32 addr);

//페이지 디렉토리를 PDTR 레지스터에 세트한다
void SetPageDirectory(UINT32 dir);

