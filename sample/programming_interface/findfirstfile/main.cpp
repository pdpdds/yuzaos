#include <minwindef.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <winapi.h>

void GetFileList(const char* szFolder)
{
    HANDLE hFind;
    WIN32_FIND_DATA data;

    hFind = FindFirstFile(szFolder, &data);

    if (hFind != INVALID_HANDLE_VALUE) 
    {
        do 
        {
            printf("%s\n", data.cFileName);
        } 
        while (FindNextFile(hFind, &data));
        
        FindClose(hFind);
    }
}

int main(int argc, char** argv)
{
    const char* szFolder = ".";
    GetFileList(szFolder);

    return 0;
}