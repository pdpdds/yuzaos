#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void print_usage() 
{
    printf("Usage: test_getopt.dll [ap] -w num -h num\n");
}

int main(int argc, char* argv[]) 
{
    int option = 0;
    int area = -1, perimeter = -1, width = -1, height = -1;

    while ((option = getopt(argc, argv, "apw:h:")) != -1) 
    {
        switch (option) 
        {
        case 'a': 
            area = 0;
            break;
        case 'p': perimeter = 0;
            break;
        case 'w': width = atoi(optarg);
            break;
        case 'h': height = atoi(optarg);
            break;
        default: print_usage();
            exit(EXIT_FAILURE);
        }
    }
    if (width == -1 || height == -1) 
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    // 면적계산
    if (area == 0) 
    {
        area = width * height;
        printf("Area: %d\n", area);
    }

    // 둘레계산
    if (perimeter == 0) 
    {
        perimeter = 2 * (width + height);
        printf("Perimeter: %d\n", perimeter);
    }
    return 0;
}