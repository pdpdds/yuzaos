#include <stdio.h>
#include "winapi.h"

int main(int argc, char** argv)
{
	printf("Hello World!!\n");

	char buf[256];
	DWORD result = GetEnvironmentVariable("BOOT_DRIVE", buf, 256);

	return 0;
}