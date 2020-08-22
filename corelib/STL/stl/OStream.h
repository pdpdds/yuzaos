#ifndef __OSTREAM_H__
#define __OSTREAM_H__

namespace std
{
//---------------OStream:------------------------

#define MAX_MESSAGE_BUFFER 256
	const char endl = '\n';

	class ostream
	{
		public:
			ostream();			

			ostream & operator<<(char *cp);
			ostream& operator<<(char c);
			ostream& operator<<(int value);
			ostream& operator<<(float value);
			ostream& operator<<(unsigned long value);
			ostream& operator<<(unsigned char *cq);
			ostream& operator<<(unsigned char c1);
			ostream& ostream::operator<<(const char *cp);

			const char* str()
			{
				return m_aDataBuffer;
			}

		private:
			char m_aDataBuffer[MAX_MESSAGE_BUFFER];
			unsigned short m_usDataSize;
	};

	extern ostream cout;
}

#endif

