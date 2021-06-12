#include "yuza.h"
#include <backends/fs/yuza/yuza-fs-factory.h>

#define DEFAULT_CONFIG_FILE "scummvm.ini"

void OSystem_YUZA::init() {
	// Initialize File System Factory
	//20200709
	//_fsFactory = new WindowsFilesystemFactory();
	_fsFactory = new YuzaFilesystemFactory();
	// Create Win32 specific window
	//_window = new SdlWindow_Win32();

#if defined(USE_TASKBAR)
	// Initialize taskbar manager
	_taskbarManager = new Win32TaskbarManager((SdlWindow_Win32 *)_window);
#endif

#if defined(USE_SYSDIALOGS)
	// Initialize dialog manager
	_dialogManager = new Win32DialogManager((SdlWindow_Win32 *)_window);
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}
