#include <minwindef.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

int main(int argc, char* argv[]) 
{
    struct dirent* pDirent;
    DIR* pDir;

    if (argc != 2) 
    {
        printf("Usage: iotest <dirname>\n");
        return 1;
    }

    pDir = opendir(argv[1]);
    if (pDir == NULL) 
    {
        printf("Cannot open directory '%s'\n", argv[1]);
        return 1;
    }

    while ((pDirent = readdir(pDir)) != NULL) 
    {
        if(S_ISDIR(pDirent->dwAttribute))
            printf("[%s (DIR)]\n", pDirent->d_name);
    }

    closedir(pDir);

    return 0;
}