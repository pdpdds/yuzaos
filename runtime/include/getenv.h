#pragma once
#ifdef __cplusplus
extern "C" {
#endif
	char *getenv(const char *name);
	int putenv(char *string);
	int setenv(const char *name, const char *value, int rewrite);
	int unsetenv(const char *name);
#ifdef __cplusplus
}
#endif