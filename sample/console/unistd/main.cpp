#include <stdio.h>         // puts()
#include <string.h>        // strlen(), memset()
#include <fcntl.h>         // O_RDWR, O_CREAT
#include <unistd.h>        // write(), close(), dup()

/*#define  BUFF_SIZE   1024

int main()
{
	int   fd_wr;
	int   fd_rd;
	char  buff[BUFF_SIZE];
	char* str = "forum.falinux.com";

	fd_wr = open("./test5.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	fd_rd = dup(fd_wr);

	write(fd_wr, str, strlen(str));

	close(fd_wr);                        

	lseek(fd_rd, 0, SEEK_SET);
	memset(buff, '\0', BUFF_SIZE);
	read(fd_rd, buff, BUFF_SIZE);

	printf("%s\n", buff);

	close(fd_rd);

	return 0;
}*/

#define  BUFF_SIZE   1024

int main()
{
	int   fd;
	char* buff = "forum.falinux.com";

	fd = open("./test.txt", O_WRONLY | O_CREAT, 0644);
	write(fd, buff, strlen(buff));
	ftruncate(fd, 10);              // 파일 디스크립터로 파일 크기 조정

	close(fd);

	return 0;
}