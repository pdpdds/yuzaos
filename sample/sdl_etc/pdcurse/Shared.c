#include <string.h>
#include <memory.h>
#include <ctype.h>

int echo_status = 1;

// STRING TRIM COURTESY OF jkramer
// https://stackoverflow.com/a/123724
void strtrim(char* string) {
	char * p = string;
	int l = strlen(p);

	while(isspace(p[l - 1])) p[--l] = 0;
	while(* p && isspace(* p)) ++p, --l;

	memmove(string, p, l + 1);
}

char* concat(char* s1, char* s2) {
	char* new = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(new, s1);
	strcat(new, s2);
	return new;
}