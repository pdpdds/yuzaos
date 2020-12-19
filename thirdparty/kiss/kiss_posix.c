/*
  kiss_sdl widget toolkit
  Copyright (c) 2016 Tarvo Korrovits <tkorrovi@mail.com>

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would
     be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not
     be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.

  kiss_sdl version 1.2.0
*/

#include "kiss_sdl.h"

char *kiss_getcwd(char *buf, int size)
{
	return getcwd(buf, size);
}

int kiss_chdir(char *path)
{
	return chdir(path);

}

int kiss_getstat(char *pathname, kiss_stat *buf)
{
	return fstat(pathname, buf);

}

/* Search pattern must end with * in Visual C */
kiss_dir *kiss_opendir(char *name)
{

	return opendir(name);

}

kiss_dirent *kiss_readdir(kiss_dir *dirp)
{

	return readdir(dirp);

}

int kiss_closedir(kiss_dir *dirp)
{
	return closedir(dirp);

}

int kiss_isdir(kiss_stat s)
{
	return s.st_mode == 0;

}

int kiss_isreg(kiss_stat s)
{
	return s.st_mode == 1;

}
 
