#include "windef.h"
#include "getenv.h"
#include "string.h"
#include "memory.h"
#include <kmalloc.h>

/* A NULL environment, the day we support env
* variables this should be in TLS and initialized */
static char* _GlbEnvironmentNull[] = { NULL };
char** _GlbEnviron = &_GlbEnvironmentNull[0];

char* __findenv(register const char* name, int* offset)
{
	register int len;
	register const char* np;
	register char** p, * c;

	char** environ = _GlbEnviron;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (np = name; *np && *np != '='; ++np)
		continue;
	len = np - name;
	for (p = environ; (c = *p) != NULL; ++p)
		if (strncmp(c, name, len) == 0 && c[len] == '=') {
			*offset = p - environ;
			return (c + len + 1);
		}
	return (NULL);
}

/* Get environmental var
* Returns the settings for a given key */
char* getenv(const char* name)
{
	/* Get a pointer to the environment
	* first entry  */
	char*** Env = &_GlbEnviron;
	register int len;
	register char** p;
	const char* c;

	/* Sanitize the env variable, the first entry
	* may not be null actually */
	if (!*Env) {
		return NULL;
	}

	/* Set inital state */
	c = name;
	while (*c && *c != '=')  c++;

	/* Identifiers may not contain an '=', so cannot match if does */
	if (*c != '=') {
		len = c - name;
		for (p = *Env; *p; ++p) {
			if (!strncmp(*p, name, len)) {
				if (*(c = *p + len) == '=') {
					return (char*)(++c);
				}
			}
		}
	}

	/* Not found */
	return NULL;
}


/*
* setenv --
*	Set the value of the environmental variable "name" to be
*	"value".  If rewrite is set, replace any current value.
*/
int
setenv(const char* name, const char* value, int rewrite)
{
	static char** lastenv;			/* last value of environ */
	char* C;
	int l_value, offset;
	if (*value == '=')			/* no `=' in value */
		++value;
	l_value = strlen(value);
	if ((C = __findenv(name, &offset))) {	/* find if already exists */
		if (!rewrite)
			return (0);
		if ((int)strlen(C) >= l_value) {	/* old larger; copy over */
			while ((*C++ = *value++))
				;
			return (0);
		}
	}
	else {					/* create new slot */
		size_t cnt;
		char** P;
		for (P = _GlbEnviron; *P != NULL; P++)
			;
		cnt = P - _GlbEnviron;
		P = (char**)krealloc(lastenv, sizeof(char*) * (cnt + 2));
		if (!P)
			return (-1);
		if (lastenv != _GlbEnviron)
			memcpy(P, _GlbEnviron, cnt * sizeof(char*));
		lastenv = _GlbEnviron = P;
		offset = cnt;
		_GlbEnviron[cnt + 1] = NULL;
	}
	for (C = (char*)name; *C && *C != '='; ++C)
		;				/* no `=' in name */
	if (!(_GlbEnviron[offset] =			/* name + `=' + value */
		(char*)kmalloc((size_t)((int)(C - name) + l_value + 2))))
		return (-1);
	for (C = _GlbEnviron[offset]; (*C = *name++) && *C != '='; ++C)
		;
	for (*C++ = '='; (*C++ = *value++); )
		;
	return (0);
}
/*
* unsetenv(name) --
*	Delete environmental variable "name".
*/
int
unsetenv(const char* name)
{
	char** P;
	int offset;
	while (__findenv(name, &offset))	/* if set multiple times */
		for (P = &_GlbEnviron[offset];; ++P)
			if (!(*P = *(P + 1)))
				break;
	return 0;
}

int putenv(char* str)
{
	char* p, * equal;
	int rval;
	if ((p = strdup(str)) == NULL)
		return (-1);
	if ((equal = strchr(p, '=')) == NULL) {
		(void)kfree(p);
		return (-1);
	}
	*equal = '\0';
	rval = setenv(p, equal + 1, 1);
	(void)kfree(p);
	return (rval);
}


