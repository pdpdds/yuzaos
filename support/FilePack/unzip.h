#ifndef _unzip_
#define _unzip_

class	File;

class	Unzip
{
public :
	Unzip();
	~Unzip();

	bool Open(File * fp);

	File * OpenFileFromZip(const char *filename);

private :
	static int FindSig(File * fp, const char *sig, int len);
	static File * OpenFileFromZip(File *fp, int offset);
	File * m_fp;
	char * m_header;
	int m_headerlen;
} ;

#endif