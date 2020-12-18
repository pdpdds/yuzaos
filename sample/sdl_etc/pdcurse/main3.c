#include <minwindef.h>
#include <curses.h>
#include <stdlib.h>


#include <dirent.h>



#include "Errors.h"
#include "Shared.c"
#include "Terminal.c"
#include "commands/cmd.c"
//#include "commands/dir.c"
#include "commands/echo.c"
#include "commands/exit.c"
#include "commands/cd.c"
#include "commands/cls.c"
#include "commands/pause.c"
//#include "commands/mkdir.c"
#include "commands/rmdir.c"

int main3(int argc, char* argv[]) {
	initscr();
	scrollok(stdscr, TRUE);

	Terminal* terminal = Terminal_create();

	// input/output
	Terminal_add_command(terminal, Command_create("pause", pause_execute));
	Terminal_add_command(terminal, Command_create("cls", cls_execute));
	Terminal_add_command(terminal, Command_create("echo", echo_execute));

	// file system
	//Terminal_add_command(terminal, Command_create("dir", dir_execute));
	Terminal_add_command(terminal, Command_create("cd", cd_execute));
	//Terminal_add_command(terminal, Command_create("mkdir", mkdir_execute));
	//Terminal_add_command(terminal, Command_create("md", mkdir_execute));
	Terminal_add_command(terminal, Command_create("rmdir", rmdir_execute));
	Terminal_add_command(terminal, Command_create("rd", rmdir_execute));

	// terminal
	Terminal_add_command(terminal, Command_create("cmd", cmd_execute));
	Terminal_add_command(terminal, Command_create("exit", exit_execute));

	// Run command prompt
	Terminal_execute_command(terminal, "cmd");

	return EXIT_SUCCESS;
}