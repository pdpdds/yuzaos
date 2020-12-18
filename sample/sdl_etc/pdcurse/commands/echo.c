void echo_execute(struct Terminal* terminal,struct Command* self, char* arguments) {
	if(strcmp(arguments, "on") == 0) {
		printw("ECHO is on.\n");
		echo_status = 1;
		return;
	} else if(strcmp(arguments, "off") == 0) {
		echo_status = 0;
		return;
	}
	
	printw("%s\n", arguments);
}