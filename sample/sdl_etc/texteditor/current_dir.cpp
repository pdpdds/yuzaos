#include "current_dir.hpp"
#include "to_utf16.hpp"
#include <unistd.h>
//#include <sys/param.h>
//#include <cstdlib>

eastl::string getCurrentDir()
{
	char tmp[MAXPATH];
	getcwd(tmp, MAXPATH);
	eastl::string result = tmp;
	free(tmp);
	return result;
}
