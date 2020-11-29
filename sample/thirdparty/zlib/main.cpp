#include <stdio.h>
#include <sprintf.h>
#include <string.h>
#include <zlib.h>

int main(int argc, char** argv)
{
	const  int BUF = 1024;
	const  int DBUF = BUF * 2 + 13;

	Bytef raw_data[] = "안녕하세요.";
	Bytef deflate_data[DBUF];

	uLong raw_size = strlen((const  char*)raw_data);
	uLong deflate_size = DBUF;

	//compress 사용하기
	{
		compress(deflate_data, &deflate_size, raw_data, raw_size);
		printf("Raw Data Size %d\n", raw_size);
		printf("Deflate Data Size: %d\n", deflate_size);
	}

	Bytef inflate_data[BUF];
	uLong inflate_size = BUF;
	//uncompress 사용하기
	{
		uncompress(inflate_data, &inflate_size, deflate_data, deflate_size);
		printf("Deflate Data Size: %d\n", deflate_size);
		printf("Inflate Size: %d\n", inflate_size);
		inflate_data[inflate_size] = NULL;
		printf("Original Data : %s\n", (const  char*)inflate_data);
	}

	return 0;
}