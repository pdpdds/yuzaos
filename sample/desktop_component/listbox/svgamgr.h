#pragma once
#include <windef.h>
#include <systemcall_impl.h>
#include "svgagui.h"
#include <skyoswindow.h>

void UpdateSheet();
bool CreateSheet(QWORD windowId, int width, int height, int colors);
void SVGA_SetEvent(const EVENT& stReceivedEvent);


