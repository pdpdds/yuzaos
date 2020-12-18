#include "Path.h"
#include <string.h>
#include <memory.h>
#include <ctype.h>

Path* Path_create(char* name) {
	Path* path = (Path*)malloc(sizeof(Path));
	path->name = name;
	path->next = 0;
	path->previous = 0;
	return path;
}

char* Path_to_string(Path* self) {
	size_t length = strlen(self->name);
	Path* current = self->next;
	while(current != 0) {
		length += strlen(current->name) + 1;
		current = current->next;
	}
	
	char* string = malloc(length);
	strcpy(string, self->name);
	current = self;
	while(current->next != 0) {
		current = current->next;
		strcat(string, current->name);
		strcat(string, "/");
	}
	string[length] = '\0';
	
	return string;
}