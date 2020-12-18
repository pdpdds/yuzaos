void rename_execute(struct Terminal* terminal, struct Command* self, char* arguments) {
	char* first = strsep(arguments, " ");
	char* second = strsep(NULL, " ");
	
	if(first == NULL || second == NULL) {
		printw(SYNTAX_ERR);
		return;
	}
	
	if(rename(first, second) == -1) {
		if(errno == EBUSY) {
			printw(BUSY_PROCESS_ERR);
		} else if (errno == EPERM || errno == EACCES) {
			printw(ACCESS_DENIED_ERR);
		} else if (errno == EEXIST || errno == ENOTEMPTY) {
			printw(NOT_EMPTY_DIRECTORY_ERR);
		}
	}
}