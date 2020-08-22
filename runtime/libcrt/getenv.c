#include "windef.h"
#include "getenv.h"
#include "stringdef.h"
#include "memory.h"
#include <systemcall_impl.h>

int errno = 0;
/* Get environmental var
* Returns the settings for a given key */
char *getenv(const char *name)
{
	static char buf[256];
	int count = Syscall_GetEnvironmentVariable(name, buf, 256);

	if (count > 0)
		return buf;

	return 0;
}


/*
* setenv --
*	Set the value of the environmental variable "name" to be
*	"value".  If rewrite is set, replace any current value.
*/
int setenv(const char *name, const char *value, int rewrite)
{
	return Syscall_SetEnvironmentVariable(name, value);
}
/*
* unsetenv(name) --
*	Delete environmental variable "name".
*/
int unsetenv(const char *name)
{
	return Syscall_SetEnvironmentVariable(name, 0);
}

int putenv(char *str)
{
	char *p, *equal;
	int rval;
	if ((p = strdup(str)) == NULL)
		return (-1);
	if ((equal = strchr(p, '=')) == NULL) {
		(void)free(p);
		return (-1);
	}
	*equal = '\0';
	rval = setenv(p, equal + 1, 1);
	(void)free(p);
	return (rval);
}


