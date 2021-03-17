/**
* This is an example that shows of the widgets present in
* Guichan. The example uses the SDL back end.
*/
#include "guichan.hpp"
#include <iostream>
#include "Excpt.h"

__declspec(noreturn) extern "C" void _stdcall _CxxThrowException(void* pExceptionObject, _ThrowInfo * pThrowInfo)
{

}

extern "C"  __declspec(naked)  EXCEPTION_DISPOSITION __cdecl __CxxFrameHandler3(

	EXCEPTION_REGISTRATION_RECORD * pRN,
	PCONTEXT pContext,
	PVOID pDC)
{

	__asm {

		ret     0
	}
}

// Here we store a global Gui object.  We make it global
// so it's easily accessable. Of course, global variables
// should normally be avioded when it comes to OOP, but
// this examples is not an example that shows how to make a 
// good and clean C++ application but merely an example
// that shows how to use Guichan.
namespace globals
{
	gcn::Gui* gui;
}

// Include code to set up an SDL application with Guichan.
// The sdl.hpp file is responsible for creating and deleting
// the global Gui object.
#include "sdl.hpp"
// Include code to set up a Guichan GUI with all the widgets
// of Guichan. The code populates the global Gui object.
#include "widgets.hpp"

//#pragma comment(lib, "sdl.lib")
//#pragma comment(lib, "sdl_image.lib")
//#pragma comment(lib, "guichan.lib")

int main(int argc, char* argv[])
{
	Exc::SetFrameHandler(true);
	Exc::SetThrowFunction(true);

	try
	{
		sdl::init();
		widgets::init();
		sdl::run();
		widgets::halt();
		sdl::halt();
	}
	// Catch all Guichan exceptions.
	catch (gcn::Exception e)
	{
		//std::cerr << e.getMessage() << std::endl;
		return 1;
	}
	// Catch all Std exceptions.
	//catch (std::exception e)
	//{
		//std::cerr << "Std exception: " << e.what() << std::endl;
		//return 1;
	//}
	// Catch all unknown exceptions.
	catch (...)
	{
		//std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}
