
/* Engesn (Derived) Operating System
 * Copyright (c) 2002, 2003 Stephen von Takach
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software
 * is hereby granted, provided that both the copyright notice and 
 * this permission notice appear in all copies of the software, 
 * derivative works or modified versions. 
*/

#include "IStream.h"
#include <memory.h>
#include <stdio.h>

/*extern char GetKeyboardInput();

namespace std
{
	istream cin;

	unsigned int istream::getString()
	{						//Acts like a console, prints char's as they are typed
		char input = '\0';	// and returns on 'Enter'. Text in IStream buffer.
		unsigned int control = 0;

		while(1)
		{
			input = GetKeyboardInput();

			if(input == '\r')
			{
				if(control >= (BUFFSIZE - 2))
				{
					control++;
				}
				buffer[control] = '\0';
				return control;
			}
			else if((input == 8) && (control > 0))
			{
				control--;
				cout << input;
			}
			else if(input >= ' ')
			{
				buffer[control] = input;
				if(control < (BUFFSIZE - 2))
				{
					control++;
				}
				else
				{
					cout << (char)8;
				}
				cout << input;
			}
		}
	}

	void istream::getWord(char *str1, char *p1)
	{
		//Obtains first word in a string

		if(*str1 != '\0')
		{
			while(*str1 != ' ')
			{
				if(*str1 == '\0')
				{ 
					*p1 = '\0';
					break;
				}
				*p1 = *str1;
				*p1++;
				*str1++;
			}
		}
		else
		{
			*p1 = '\0';
		}
	}

	istream& istream::operator >> (char *cp)
	{
		getString();			//Allows input
		getWord(buffer, cp);	//Obtains first word
		cout << '\n';			//Moves cursor down a line
		return *this;
	}

	istream& istream::operator >> (char &c)
	{
		getString();			//Allows input
		c = buffer[0];			//Obtains first char
		cout << '\n';			//Moves cursor down a line
		return *this;
	}

	istream& istream::operator >> (unsigned char *ucp)
	{
		getString();			//Allows input
		getWord(buffer, (char *)ucp);	//Obtains first word
		cout << '\n';			//Moves cursor down a line
		return *this;
	}

	istream& istream::operator >> (unsigned char &uc)
	{
		getString();			//Allows input
		uc = (unsigned char)buffer[0];	//Obtains first char
		cout << '\n';			//Moves cursor down a line
		return *this;
	}
};
*/

namespace std
{
	istream::istream(const char* buf)
	{
		m_mode = oct;
		pos = 0;
		memcpy(buffer, buf, sizeof(buf));
	}

	void istream::getWord(char *str1, char *p1)
	{
	
		if (*str1 != '\0')
		{
			while (*str1 != ' ')
			{
				if (*str1 == '\0')
				{
					*p1 = '\0';
					break;
				}
				*p1 = *str1;
				*p1++;
				*str1++;
				pos++;
			}
		}
		else
		{
			*p1 = '\0';
		}
	}

	istream& istream::operator >> (char *cp)
	{	
		getWord(buffer+ pos, cp);	
		return *this;
	}

	istream& istream::operator >> (char &c)
	{		
		c = buffer[pos];
		pos++;
		return *this;
	}

	istream& istream::operator >> (unsigned char *ucp)
	{	
		getWord(buffer+ pos, (char *)ucp);
		return *this;
	}

	istream& istream::operator >> (int& num)
	{
		char ucp[256];
		getWord(buffer, (char *)ucp);
		num = atoi(ucp);

		return *this;
	}

	istream& istream::operator >> (double& value)
	{
		char ucp[256];
		getWord(buffer, (char *)ucp);
		value = (double)atof(ucp);

		return *this;
	}	

	istream& istream::operator >> (unsigned char &uc)
	{	
		uc = (unsigned char)buffer[0];	//Obtains first char
		cout << '\n';			//Moves cursor down a line
		return *this;
	}

//////////////////////////////////
	int istream::tellg()
	{
		return pos;
	}

	void istream::seekg(int loc, int mode)
	{
		switch (mode)
		{
		case _beg:
			pos = 0 + loc;
			break;
		case _cur:
			pos = pos + loc;
			break;
		case _end:
			pos = pos - loc;			
			break;

		}
	}

	int istream::getline(char *str, int length, char skip_char)
	{
		int _length = 0;
		while (*(buffer + pos) != '\n')
		{
			if (*(buffer + pos) == skip_char)
			{
				pos++;
				continue;
			}
			*str = *(buffer + pos);
			str++;
			pos++;
			_length++;
		}

		return _length;
	}
};

