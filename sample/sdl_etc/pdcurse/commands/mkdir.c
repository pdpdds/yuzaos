void mkdir_execute(struct Terminal* terminal,struct Command* self, char* arguments) {
	char* dir_name = arguments; // TODO implement argument parsing
	if(strlen(dir_name) == 0) {
		printw(SYNTAX_ERR);
	} else if(mkdir(dir_name, 777) == -1) {
		if(errno == EBUSY) {
			printw(BUSY_PROCESS_ERR);
		} else if (errno == EPERM || errno == EACCES) {
			printw(ACCESS_DENIED_ERR);
		} else if (errno == EEXIST || errno == ENOTEMPTY) {
			printw(NOT_EMPTY_DIRECTORY_ERR);
		}
	}
}