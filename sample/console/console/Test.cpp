#include "Test.h"
//#include "snappy.h"
#include <slog.h>
#include <string.h>
#include <math.h>
//#include "ModuleManager.h"
//#include <SystemAPI.h>

void TestSnappy()
{
	/*const char* input = "simple test data gffdsdfgdfgsssssssssssssssssssssssssssssssssssssssssssssssssssssss";
	std::string output;
	int size = snappy::Compress(input, strlen(input) + 1, &output);
	printf("%d %d\n", strlen(input), size);*/
}

void Testslog()
{

	/* Used variables */
	char char_arg[32];
	strcpy(char_arg, "test string");
	int int_arg = 69;
	
	/* Greet users */
	//greet();

	/* Initialize slog with default parameters */
	slog_init("example", 0, 3, 0);
	
	/* Log and print something with level 0 */
	slog(0, SLOG_LIVE, "Test message with level 0");
	
	/* Log and print something with level 1 */
	slog(1, SLOG_WARN, "Warn message with level 1");

	/* Log and print something with level 2 */
	slog(2, SLOG_INFO, "Info message with level 2");

	/* Log and print something with level 3 */
	slog(3, SLOG_LIVE, "Test message with level 3");

	/* Log and print something with char argument */
	slog(0, SLOG_DEBUG, "Debug message with char argument: %s", char_arg);

	/* Log and print something with int argument */
	slog(0, SLOG_ERROR, "Error message with int argument: %d", int_arg);

	/* Test log with higher level than log max value
	* This will never be printed while log level argument is higher than max log level */
	slog(4, SLOG_NONE, "[LIVE] Test log with higher level than log max value");
	
	/* On the fly change parameters (enable file logger) */
	SlogConfig slgCfg;
	slog_config_get(&slgCfg);
	slgCfg.nToFile = 1;
	slog_config_set(&slgCfg);

	// Print message and save log in the file 
	slog(0, SLOG_DEBUG, "Debug message in the file with int argument: %d", int_arg);

	// On the fly change parameters (enable file logger) 
	slog_config_get(&slgCfg);
	slgCfg.nPretty = 1;
	slog_config_set(&slgCfg);

	// Print message and save log in the file without colorization
	slog(0, SLOG_DEBUG, "Debug message with disabled pretty log");
}
/*
typedef int(*PMain)(void*);
typedef CSysfs*(*PCreateDrive)(char* fileName);

void TestLibFAT32()
{
	void* libDriveHandle = (void*)ModuleManager::GetInstance()->LoadDLL("libfat32.dll");
	PCreateDrive func = (PCreateDrive)ModuleManager::GetInstance()->GetModuleFunction(libDriveHandle, "CreateDrive");
	CSysfs* pDrive = func("fat32.img");
	if (pDrive)
	{
	pDrive->CreateDirectory("\\juhang3", 0);
	//pDrive->FileOpen(&file, "autoexec.bat", 0);
	}
}*/

void TestMath()
{
	double value = 15.0f;
	float a = 2.0f;
	value = sin(a);
	printf("sin. %f, %f\n", a, a);
	/*while (1)
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


	double value, ipart, fpart;
	value = 3.14;
	fpart = modf(value, &ipart);
	printf("%f %f %f\n", value, ipart, fpart);*/
}


void TestFile()
{
	char	commandBuffer[MAXPATH];

	FILE* fp = fopen("test.txt", "rb");

	memset(commandBuffer, 0, MAXPATH);
	fread(commandBuffer, MAXPATH, 1, fp);
	printf("%s\n", commandBuffer);
	fclose(fp);


	//char* pAddress = (char*)Syscall_VirtualAlloc(NULL, 4096, 0, 0);
	//printf("%x\n", pAddress);
}

void Test()
{

	//TestStart();
	//TestSoundBlaster();
	//TestSnappy();
	//Testslog();
	//TestLibFAT32();
	//TestMath();
	//TestFile();
}
