#include <string.h>
#include <math.h>

int main(int argc, char* argv[]) 
{
	char	commandBuffer[MAXPATH];

	FILE* fp = fopen("test.txt", "rb");

	memset(commandBuffer, 0, MAXPATH);
	fread(commandBuffer, MAXPATH, 1, fp);
	printf("%s\n", commandBuffer);
	fclose(fp);

	//char* pAddress = (char*)Syscall_VirtualAlloc(NULL, 4096, 0, 0);
	//printf("%x\n", pAddress);

    return 0;
}