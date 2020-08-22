#pragma once
#include <stdio.h>

//FILE IO INTERFACE
typedef struct tag_FILE_IO_INTERFACE
{
	unsigned char(*sky_disk_initialize)();
	unsigned char(*sky_disk_read)(unsigned char* buff, DWORD sector, UINT count);
	unsigned char(*sky_disk_write)(const unsigned char* buff, DWORD sector, UINT count);
	unsigned char(*sky_disk_status)();
	bool(*sky_disk_info)(void* pInfo);

} FILE_IO_INTERFACE;

//WIN32
typedef struct tag_WIN32_FILE_IO_INTERFACE
{
	size_t(*sky_fread)(void* ptr, size_t size, size_t nmemb, FILE* stream);
	FILE* (*sky_fopen)(const char* filename, const char* mode);
	size_t(*sky_fwrite)(const void* ptr, size_t size, size_t nmemb, FILE* stream);
	int (*sky_fseek)(FILE* stream, long int offset, int whence);
	int (*sky_fclose)(FILE* stream);

} WIN32_FILE_IO_INTERFACE;
