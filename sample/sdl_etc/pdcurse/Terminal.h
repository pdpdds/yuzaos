#include <stddef.h>
#include "Command.h"
#include "Path.h"

struct Terminal {
	
	Command** commands;
	size_t length;
	size_t capacity;
	
	Path* first_path;
	Path* last_path;
	
};
typedef struct Terminal Terminal;

Terminal* Terminal_create();
int Terminal_add_command(Terminal* self, Command* command);
void Terminal_execute_command(Terminal* self, char* name);

void Terminal_navigate(Terminal* self, char* path_name);
void Terminal_navigate_folder(Terminal* self, char* path_name);
inline char* Terminal_get_path(Terminal* self);
inline char* Terminal_get_subdir(Terminal* self, char* subdir);