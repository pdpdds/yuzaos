#include <stdio.h>
#include <wchar.h>

int main(int argc, char** argv)
{
	wchar_t* wcs = L"3.1415926This stopped it";
	wchar_t* stopwcs;

	printf("wcs = \"%ls\"\n", wcs);
	printf("   wcstod = %f\n", wcstod(wcs, &stopwcs));
	printf("   Stop scanning at \"%ls\"\n", stopwcs);

	/*printf("   wcstof = %f\n", wcstof(wcs, &stopwcs));
	printf("   Stop scanning at \"%ls\"\n", stopwcs);
	printf("   wcstold = %lf\n", wcstold(wcs, &stopwcs));
	printf("   Stop scanning at \"%ls\"\n", stopwcs);*/
	

	return 0;
}