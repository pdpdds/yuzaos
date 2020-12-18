#include <systemcall_impl.h>

void exit_execute(struct Terminal* terminal, struct Command* self, char* arguments) {
	// switch back from curses mode
	endwin();
	
	// free all the memories
	size_t i;
	for(i = 0; i < terminal->length; i++) {
		free(terminal->commands[i]);
	}
	free(terminal->commands);
	
	Path* path = terminal->first_path;
	while(path != NULL) {
		free(path);
		path = path->next;
	}
	
	free(terminal);
	
	// exit
	Syscall_exit(EXIT_SUCCESS);
}