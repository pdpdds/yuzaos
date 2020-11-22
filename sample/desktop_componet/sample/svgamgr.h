#pragma once
#include <skyoswindow.h>
#include <string.h>
#include <systemcall_impl.h>
#include "svgagui.h"


void UpdateSheet();
bool CreateSheet(QWORD windowId, int width, int height, int colors);
void SVGA_SetEvent(const EVENT& stReceivedEvent);


