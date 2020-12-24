#include <minwindef.h>
#include <yuzaapi.h>
#include <ConsoleManager.h>
#include <GUIConsoleFramework.h>
#include <time.h>
#include <cli.h>
#include <systemcall_impl.h>
#include <tinydir.h>
/**
 * Callback for 'help' command.
 */
static int cmd_help(int argc, char** argv)
{
	printf("Help Command\n");
	return 0;
}


/**
 * Callback for 'time' command.
 */
static int cmd_time(int argc, char** argv)
{
	time_t t = time(NULL);
	printf("Current time: %s", asctime(localtime(&t)));
	return 0;
}


static int cmd_dir(int argc, char** argv)
{
	char szCurrentDir[MAXPATH];
	Syscall_GetCurrentDirectory(MAXPATH, szCurrentDir);

	tinydir_dir dir;
	tinydir_open(&dir, szCurrentDir);

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);

		printf("[%s] %s", file.is_dir ? "DIR" : "FILE", file.name);
		if (file.is_dir)
		{
			printf("/");
		}
		printf("\n");

		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	return 0;
}

/*static int cmd_dir(int argc, char** argv)
{
	char szCurrentDir[MAXPATH];
	Syscall_GetCurrentDirectory(MAXPATH, szCurrentDir);

	dir_information* pInfo = open_directory(szCurrentDir);

	if (pInfo)
	{
		char entryName[CROSS_LEN];
		bool is_directory = false;
		bool result = read_directory_first(pInfo, entryName, is_directory);

		while (result)
		{
			printf("[%s] %s\n", is_directory ? "DIR" : "FILE", entryName);
			result = read_directory_next(pInfo, entryName, is_directory);
		}

		close_directory(pInfo);
	}

	return 0;
}*/

extern char* rtrimslash(char* str);
static int cmd_chdir(int argc, char** argv)
{
	if (argc != 2)
		return -1;

	char szCurrentDir[MAXPATH];
	Syscall_GetCurrentDirectory(MAXPATH, szCurrentDir);

	if (strcmp(argv[1], "..") == 0)
	{
		dirname(szCurrentDir);
	}
	else
	{
		strrchr(szCurrentDir, '/');
		strcat(szCurrentDir, argv[1]);
	}
	
	struct stat test;
	if (fstat(szCurrentDir, &test) != 0 || !S_ISDIR(test.st_mode))
	{
		printf("can't change directory [%s]", szCurrentDir);
		return -1;
	}

	rtrimslash(szCurrentDir);
	strcat(szCurrentDir, "/");
	SetCurrentDirectory(szCurrentDir);

	return 0;
}



/**
 * Callback for 'test' command. (I.e 'test 1 2 3')
 */
static int cmd_test(int argc, char** argv)
{
	int i;
	for (i = 0; i < argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	return 0;
}

const cli_cmd_t commands[] = {
		{
			"help",
			"This is the Help",
			cmd_help
		},
		{
			"time",
			"Shows current time",
			cmd_time
		},
		{
			"dir",
			"displays a list of files and subdirectories in a directory.",
			cmd_dir
		},
		{
			"cd",
			"change the directory.",
			cmd_chdir
		},
		{
			"test",
			"test command arguments.",
			cmd_test
		}
};

const cli_cmd_list_t cmdlist = {
	commands,
	(sizeof(commands) / sizeof(commands[0]))
};

int main_impl(int argc, char** argv)
{
	printf("%s\nConsole Mode Start!!\n", (char*)argv[0]);
	
	CLI_Init(&cmdlist);
	CLI_Utf8(1);

	while (1)
	{
		char c;
		while (1) {
			//c = getchar();

			c = Syscall_GetChar();
			CLI_Parse(&c, 1);
			CLI_HandleLine();

		}
	}

	printf("Bye!!\n");

	return 0;
}

int main(int argc, char** argv)
{	
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}