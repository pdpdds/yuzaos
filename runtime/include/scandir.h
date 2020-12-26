#pragma once
#include <stdbool.h>
#include <minwindef.h>
#include <minwinbase.h>
#include <dirent.h>


#ifdef __cplusplus
extern "C"
{
#endif
	int scandir(const char* dirname, struct dirent*** namelist, int (*sdfilter)(struct dirent*), int (*dcomp)(const struct dirent**, const struct dirent**));
	int alphasort(const struct dirent** a, const struct dirent** b);

#ifdef __cplusplus
}
#endif