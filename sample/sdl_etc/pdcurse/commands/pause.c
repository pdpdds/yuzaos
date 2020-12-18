void pause_execute(struct Terminal* terminal,struct Command* self, char* arguments) {
	noecho();
	printw("Press any key to continue...");
	getch();
	printw("\n");
	refresh();
	echo();
}