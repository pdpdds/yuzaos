#include <string.h>

void cmd_execute(struct Terminal* terminal, struct Command* self, char* arguments) {
	//struct utsname os_stat;
	//uname(&os_stat);
	//printf("%s [Version %s]\n", os_stat.version, os_stat.release);
	//printf("GNU General Public License (version 3)\n");
	refresh();
	
	char line[256] = { 0, };
	while(1) {
		if(echo_status) {
			char* path = Path_to_string(terminal->first_path);
			printw("\n%s> ", path);
			refresh();
			//free(path);
		}
		getnstr(line, 256);
		Terminal_execute_command(terminal, line);
	}
}