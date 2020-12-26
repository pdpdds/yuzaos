#include "scandir.h"
#include <memory.h>
#include <FileService.h>
#include <sprintf.h>
#include <string.h>

/*-----------------------------------------------------------------------*/
/**
 * Scan a directory for all its entries
 * Return -1 on error, number of entries on success
 */
size_t get_dir_item_count(const char* dirname, int (*sdfilter)(struct dirent*))
{
	DIR* dirp = 0;
	int count = 0;

	struct dirent* info;
	if ((dirp = opendir(dirname)) == 0)
		return 0;
	bool result = true;
	while (result)
	{
		info = readdir(dirp);
		if (info == 0)
		{
			result = false;
			break;
		}

		if (sdfilter != NULL && !(*sdfilter)(info))
			continue;       /* just selected names */

		count++;


	}

	closedir(dirp);

	if (dirp)
	{
		free(dirp);
	}

	return count;
}

int alphasort(const struct dirent** a, const struct dirent** b)
{
	return strcoll((*a)->d_name, (*b)->d_name);
}

int scandir(const char* dirname, struct dirent*** namelist, int (*sdfilter)(struct dirent*), int (*dcomp)(const void*, const void*))
{
	struct dirent* p = NULL, ** names = NULL;
	struct dirent* info;
	size_t nitems = 0;

	size_t arraysz = get_dir_item_count(dirname, sdfilter);

	if (arraysz == 0)
		return 0;

	struct DIR* dirp;
	int result = 0;

	dirp = opendir(dirname);
	if (dirp == 0)
		goto error_out;

	names = (struct dirent**)malloc(arraysz * sizeof(struct dirent*));
	if (names == NULL)
		goto error_out;


	while (true)
	{

		info = readdir(dirp);
		if (info == 0)
		{
			break;
		}

		if (sdfilter != NULL && !(*sdfilter)(info))
			continue;       /* just selected names */


		p = (struct dirent*)malloc(sizeof(struct dirent));
		if (p == NULL)
			goto error_out;


		memcpy(p, info, sizeof(struct dirent));

		names[nitems++] = p;

		p = NULL;



	}

	closedir(dirp);

	free(dirp);

	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(struct dirent*), dcomp);

	*namelist = names;

	return nitems;

error_out:
	if (names)
	{
		int i;
		for (i = 0; i < nitems; i++)
			free(names[i]);
		free(names);
	}

	closedir(dirp);
	return -1;
}

