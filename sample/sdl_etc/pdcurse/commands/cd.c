void cd_execute(struct Terminal* terminal, struct Command* self, char* arguments) {
	if(strlen(arguments) == 0) {
		char* path = Path_to_string(terminal->first_path);
		printw("%s\n", path);
		free(path);
	} else {
		Terminal_navigate(terminal, arguments);
	}
}