#include <string.h>
#include <math.h>
#include <systemcall_impl.h>

int main(int argc, char* argv[]) 
{
	
	float a = 0.0f;
	
	while (a < 6)
	{
		printf("sin(%f), %f\n", a, sin(a));
		printf("cos(%f), %f\n", a, cos(a));
		printf("tan(%f), %f\n", a, tan(a));
		a = a + 0.1f;

		Syscall_Sleep(1000);
	}
	
	double ipart, fpart;
	double value = 3.14;
	fpart = modf(value, &ipart);
	printf("%f %f %f\n", value, ipart, fpart);

    return 0;
}