#include <string.h>
#include <math.h>

int main(int argc, char* argv[]) 
{
	char	buffer[MAXPATH];

	FILE* fp = fopen("test.txt", "rb");

	memset(buffer, 0, MAXPATH);
	fread(buffer, MAXPATH, 1, fp);
	printf("%s\n", buffer);
	fclose(fp);

    return 0;
}