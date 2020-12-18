void dir_execute(struct Terminal* terminal, struct Command* self, char* arguments) {
	//struct stat dir_stat;
	//stat(Terminal_get_path(terminal), &dir_stat);
	printw(" Volume in drive ? is ?\n");
	printw(" Volume Serial Number is ?\n");
	printw("\n");
	printw(" Directory of %s\n\n", Terminal_get_path(terminal));
	
	DIR* dir = opendir(Terminal_get_path(terminal));
	struct dirent* dp;
	struct stat file_stat;
	
	size_t files = 0;
	size_t directories = 0;
	
	size_t total_size = 0;
	
	while((dp = readdir(dir)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
		} else {
			char* absolute_path = concat(Terminal_get_path(terminal), dp->d_name);
			
			if(stat(absolute_path, &file_stat) != 0) {
				printw("%s ERROR!\n", absolute_path);
				free(absolute_path);
				return;
			}
			
			char date[18]; // 00/00/0000
			time_t timestamp = (time_t) file_stat.st_ctime;
			struct tm* tm_info = localtime(&timestamp);
			
			strftime(date, 18, "%d/%m/%Y  %I:%M", tm_info);
			printw("%s  %8s %8d %s\n", date, S_ISREG(file_stat.st_mode) ? "<DIR> " : "" , file_stat.st_size, dp->d_name);
			
			if(S_ISREG(file_stat.st_mode)) {
				files++;
			} else {
				directories++;
			}
			total_size += file_stat.st_size;
			
			free(absolute_path);
		}
	}
	
	struct statvfs fs_stat;
	statvfs("/", &fs_stat);
	long free_size = fs_stat.f_bavail * fs_stat.f_bfree;
	
	//               7
	//     5,623,325
	
	printw("%16d File(s) %16lu bytes\n", files, total_size);
	printw("%16d  Dir(s) %16lu bytes free\n", directories, free_size);
}