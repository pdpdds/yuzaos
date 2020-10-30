#include <string.h>
#include <snappy.h>

int main(int argc, char* argv[]) 
{
	const char* input = "YuzaOS. simple Test data";
	std::string output;
	int size = snappy::Compress(input, strlen(input) + 1, &output);
	printf("Input Data Size : %d. Compressed Size : %d\n", strlen(input) + 1, size);

    return 0;
}