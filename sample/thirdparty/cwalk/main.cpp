#include <cwalk.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    char result[FILENAME_MAX];

    // The following function cleans up the input path and writes it
    // to the result buffer.
    cwk_path_normalize("/var/log/weird/////path/.././..///", result,
        sizeof(result));

    printf("%s\n", result);
    return EXIT_SUCCESS;
}