#ifndef __ISTREAM_H__
#define __ISTREAM_H__

#include "OStream.h"


namespace std
{
//---------------IStream:------------------------

	const int BUFFSIZE = 256;

#define _beg 0
#define _cur 1
#define _end 2

	class istream
	{

		public:
			enum
			{
				dec = 0,
				oct = 1,
				hex = 2,
			};

			istream(const char* buf);

			istream & operator >> (char *cp);
			istream& operator >> (char &c);
			istream& operator >> (unsigned char *ucp);
			istream& operator >> (unsigned char &uc);
			istream& operator >> (int &uc);

			istream& operator >> (double &value);
			

			//unsigned int getString();
			void getWord(char *str1, char *p1);
			int getline(char *buffer, int length, char skip_char);

			int tellg();
			void seekg(int loc, int mode);

			void flags(int mode)
			{
				m_mode = mode;
			}
			

		private:
			char buffer[BUFFSIZE];	
			int pos;
			int m_mode;
	};	
}

#endif

