gcc `sdl-config --cflags` -Wall -c test.c
gcc `sdl-config --libs` -o test test.o
