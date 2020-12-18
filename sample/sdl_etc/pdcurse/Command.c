#include "Command.h"
#include <windef.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

Command* Command_create(char* name, void (*execute)(struct Terminal* terminal, struct Command* self, char* arguments)) {
	Command* command = (Command*)malloc(sizeof(Command));
	command->name = name;
	command->execute = execute;
	return command;
}