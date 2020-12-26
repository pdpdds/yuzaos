
#include "../../SDL_internal.h"
#include "../../../include/SDL_messagebox.h"
#include "../../video/SDL_sysvideo.h"
/* This function is called if a Task Dialog is unsupported. */
int YUZA_ShowMessageBox(_THIS, const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    return 0;
}