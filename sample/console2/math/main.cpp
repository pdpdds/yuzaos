#include <string.h>
#include <math.h>
#include <systemcall_impl.h>

int main(int argc, char* argv[]) 
{
	double value = 15.0f;
	float a = 2.0f;
	value = sin(a);
	printf("sin. %f, %f\n", a, a);
	while (1)
	{
		//printf("cos. %f, %f\n", a, cos(a));

		if (a >= 2.4)
		{
			int j = 1;
		}
		printf("sin. %f, %f\n", a, sin(a));
		//printf("tan. %f, %f\n", a, tan(a));
		a = a + 0.1f;
		Syscall_Sleep(1000);
	}
	cos(180);

	double ipart, fpart;
	value = 3.14;
	fpart = modf(value, &ipart);
	printf("%f %f %f\n", value, ipart, fpart);

    return 0;
}