#ifndef _FILE_
#define _FILE_


class File
{
public :
	enum fileorigin {
		SET, CUR, END
	} ;
	virtual ~File() {}
	virtual int Read(void *ptr, int len) = 0;
	virtual void Seek(int pos, fileorigin origin) = 0;
	virtual int Tell() = 0;
	virtual int Length() = 0;
} ;

#include <stdio.h>

class	FileStreamPacked2 : public File
{
public :
	FileStreamPacked2(File *fp, int start, int len) {
		m_stream = fp;
		m_pivot = start;
		m_len = len;
	}
protected :
	int Read(void *ptr, int len) {
		if (len + m_pos > m_len)
			len = m_len - m_pos;
		if (len > 0)
		{
			m_stream->Seek(m_pivot + m_pos, SET);
			m_stream->Read(ptr, len);
		}
		m_pos += len;
		return len;
	}
	void Seek(int pos, fileorigin origin)
	{
		switch(origin)
		{
		case CUR :
			m_pos += pos; break;
		case END :
			m_pos = m_len + pos; break;
		case SET :
			m_pos = pos; break;
		}
		m_stream->Seek(m_pivot + m_pos, SET);
	}
private :
	File * m_stream;
	int m_pivot, m_len, m_pos;
} ;


//

#define MIN(A, B) ((A)<(B)?(A):(B))

class FileMan : public File
{
public :
	FileMan(int len) {
		m_pos = 0;
		m_len = len;
	}
	virtual ~FileMan() {
	}
	int Read(void *ptr, int len) {
		if (len > m_len-m_pos)
			return Read(ptr, m_len-m_pos);
		ReadMan(ptr, len);
		m_pos += len;
		return len;
	}
	void Seek(int pos, fileorigin origin) {
		if (origin == CUR)
			Seek(pos + m_pos, SET);
		else
		if (origin == END)
			Seek(pos + m_len, SET);
		else
		if (m_pos != pos) {
			SeekMan(pos);
			m_pos = pos;
		}
	}
	int Tell() {
		return m_pos;
	}
	int Length() {
		return m_len;
	}
protected :
	virtual void ReadMan(void *ptr, int len) = 0;
	virtual void SeekMan(int pos) {}
private :
	int m_pos, m_len;
} ;

#include <stdio.h>

class	FileStream : public FileMan
{
public :
	FileStream(FILE *fp, int len) : FileMan(len) {
		m_fp = fp;
	}
	~FileStream() {
		fclose(m_fp);
	}
protected :
	void ReadMan(void *ptr, int len) {
		fread(ptr, 1, len, m_fp);
	}
	void SeekMan(int pos) {
		fseek(m_fp, pos, SEEK_SET);
	}
private :
	FILE * m_fp;
} ;

class	FileStreamPacked : public FileMan
{
public :
	FileStreamPacked(File *fp, int start, int len) : FileMan(len) {
		m_stream = fp;
		m_pivot = start;
	}
protected :
	void ReadMan(void *ptr, int len) {
		m_stream->Seek(m_pivot + Tell(), SET);
		m_stream->Read(ptr, len);
	}
	void SeekMan(int pos) {
		m_stream->Seek(m_pivot + pos, SET);
	}
private :
	File * m_stream;
	int m_pivot;
} ;

#include <string.h>

class	FileMem : public FileMan
{
public :
	FileMem(void *mem, int len, bool free=true) : FileMan(len) {
		m_ptr = (unsigned char*) mem;
	}
	virtual ~FileMem() {
		if (m_free)
			delete [] m_ptr;
	}
protected :
	virtual void ReadMan(void *ptr, int len) {
		memcpy(ptr, m_ptr + Tell(), len);
	}
private :
	unsigned char *m_ptr;
	bool m_free;
} ;

#endif