#include "Support.h"
#include <string.h>

int _ismbcspace(int c)
{
	if (c == 0x20) return 1;
	else return 0;
};

unsigned char* _mbsinc(const unsigned char* current)
{
	return (unsigned char*)current + 1;
};

char* _mbsupr(char* str)
{
	int len = strlen(str);
	int i;
	for (i = 0; i < len; i++)
	{
		if (str[i] >= 0x61 && str[i] <= 0x7a)
			str[i] = str[i] - 0x20;
	}

	return str;
};

 char *_strlwr(  char * str)
 {
 	int len = strlen(str);
 	int i;
 	for (i=0; i<len; i++)
 	{
 		if(str[i]>=0x41 && str[i]<=0x5a)
 			str[i] = str[i] + 0x20;
 	}
 	
 	return str;
 };

int _stricmp(const char *string1,const char *string2 )
 {
 	int len = strlen(string1);
 	char* c1 = new char[len+1];
 	strcpy(c1 , string1);
 	c1[len] = 0;
 	_strlwr(c1);
 	
 	len = strlen(string2);
 	char* c2 = new char[len+1];
 	strcpy(c2 , string2);
 	c2[len] = 0;	
 	_strlwr(c2);	
 	
 	int ret = strcmp(c1 , c2);
 	delete c1;
 	delete c2;
 	
 	return ret;
 };
