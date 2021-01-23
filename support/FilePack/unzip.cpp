#include "file.h"
#include "unzip.h"
#include <zlib.h>
#include <string.h>

Unzip :: Unzip()
{
	m_fp = 0l;
	m_header = 0l;
}

	Unzip::~Unzip()
{
	if (m_fp)
		delete m_fp;
	if (m_header)
		delete m_header;
}

bool Unzip :: Open(File * fp)
{
	static char sig[] = { 'P', 'K', 0x05, 0x06 };
	int offset, len, headerpos;

	if (fp == 0l)
		return false;

	headerpos = FindSig(fp, sig, sizeof(sig));

	if (headerpos == -1)
		return false;

	fp->Seek(headerpos + 12, File::SET);

	fp->Read(&len, 4);
	fp->Read(&offset, 4);

	fp->Seek(offset, File::SET);

	m_header = new char [len];
	m_headerlen = len;

	fp->Read(m_header, len);

	m_fp = fp;

	return true;
}

int Unzip :: FindSig(File * fp, const char * sig, int len)
{
	unsigned char buf[32];
	unsigned long i1, i2, i;

	for(i2 = fp->Length(); i2>0;)
	{
		i1 = i2 > sizeof(buf)-len ? i2 - (sizeof(buf) - len) : 0;

		fp->Seek(i1, File::SET);
		fp->Read(buf, i2-i1);

		for(i=i1; i<i2; i++)
			if (!memcmp(&buf[i - i1], sig, len))
				return i;

		memcpy(buf + sizeof(buf) - len, buf, len);
		i2 = i1;
	}

	return -1;
}

#pragma pack(1)

struct ZIPFILEHEAD {
	unsigned long	cent_file_header_sig;
	unsigned char	version_made_by;
	unsigned char	host_os;
	unsigned char	version_needed_to_extract;
	unsigned char	os_needed_to_extract;

	unsigned short	general_purpose_bit_flag;
	unsigned short	compression_method;
	unsigned short	last_mod_file_time;
	unsigned short	last_mod_file_date;
	unsigned long	crc32;
	unsigned long	compressed_size;
	unsigned long	uncompressed_size;
	unsigned short	filename_length;
	unsigned short	extra_field_length;

	unsigned short	file_comment_length;
	unsigned short	disk_number_start;
	unsigned short	internal_file_attrib;
	unsigned long	external_file_attrib;
	unsigned long	offset_lcl_hdr_frm_frst_disk;
};

#pragma pack()

File * Unzip :: OpenFileFromZip(const char *fname)
{
	ZIPFILEHEAD * header;
	int i;

	for(i=0; i<m_headerlen;)
	{
		header = (ZIPFILEHEAD*)(&m_header[i]);

		if (strlen(fname) == header->filename_length &&
			!memcmp(fname, (void*)&header[1], header->filename_length))
		{
			return OpenFileFromZip(m_fp, header->offset_lcl_hdr_frm_frst_disk);
		}

		i += sizeof(*header) + header->filename_length +
			header->extra_field_length + header->file_comment_length;
	}

	return 0l;
}

#pragma pack(2)

struct	ZIPLOCALHEAD {
	unsigned long	signature;
	unsigned short	version;

	unsigned short	general_purpose_bit_flag;
	unsigned short	compression_method;
	unsigned short	last_mod_file_time;
	unsigned short	last_mod_file_date;
	unsigned long	crc32;
	unsigned long	compressed_size;
	unsigned long	uncompressed_size;
	unsigned short	filename_length;
	unsigned short	extra_field_length;
} ;

#pragma pack()

#ifndef _LOAD_ENTIRE_FILES_

class	FileZip : public FileMan
{
public :
	FileZip(File *fp, int len) : FileMan(len) {
		m_fp = fp;
		memset(&m_zipstream, 0, sizeof(m_zipstream));
		InitZipstream();
	}
	~FileZip() {
		delete m_fp;
		inflateEnd(&m_zipstream);
	}
protected :
	void ReadMan(void *ptr, int len) {
		m_zipstream.next_out = (unsigned char*) ptr;
		m_zipstream.avail_out = len;

		do {
			if (m_zipstream.avail_in == 0 &&
				m_zipstream.next_in == &m_buff[sizeof(m_buff)])
			{
				m_zipstream.next_in = m_buff;
				m_zipstream.avail_in = m_fp->Read(m_buff, sizeof(m_buff));
			}

			inflate(&m_zipstream, Z_NO_FLUSH);
		}	while(m_zipstream.avail_out > 0);
	}
	void SeekMan(int pivot) {
		if (pivot < Tell())
		{
			InitZipstream();
			SkipBytes(pivot);
		}	else
			SkipBytes(pivot-Tell());
	}
private :
	void InitZipstream() {
		m_fp->Seek(0, SET);
		m_zipstream.next_in = &m_buff[sizeof(m_buff)];
		m_zipstream.avail_in = 0;
		inflateInit2(&m_zipstream, -MAX_WBITS);
	}

	void SkipBytes(int len) {
		unsigned char buff[64];
		int i;
		for(i=0; i<len; i+=sizeof(buff))
			ReadMan(buff, len-i < sizeof(buff) ? len-i : sizeof(buff));
	}

	File * m_fp;
	z_stream m_zipstream;
	unsigned char m_buff[1024];
};

#endif

File * Unzip :: OpenFileFromZip(File *fp, int offset)
{
	ZIPLOCALHEAD header;

	fp->Seek(offset, File::SET);
	fp->Read(&header, sizeof(header));

	offset += sizeof(header) + header.filename_length + header.extra_field_length;

	if (header.compression_method == 0x0000)
		return new FileStreamPacked(fp, offset, header.compressed_size);

	if (header.compression_method == 0x0008)
	{
#ifndef _LOAD_ENTIRE_FILES_
		return new FileZip(new FileStreamPacked(fp, offset, header.compressed_size), header.uncompressed_size);
#else
		FileStreamPacked infile(fp, offset, header.compressed_size);
		unsigned char * ptr, buffer[2048];
		z_stream zipstream;
		unsigned long i;

		zipstream.next_in = 0;
		zipstream.avail_in = 0;
		zipstream.next_out = ptr = new unsigned char [header.uncompressed_size];
		zipstream.avail_out = header.uncompressed_size;

		inflateInit2(&zipstream, -MAX_WBITS);

		do
		{
			zipstream.next_in = buffer;
			zipstream.avail_in = infile.Read(buffer, sizeof(buffer));

			do {
				inflate(&zipstream, Z_NO_FLUSH);
			}	while(zipstream.avail_in > 0);
		}	while(zipstream.avail_out > 0);

		inflateEnd(&zipstream);

		return new FileMem(ptr, header.uncompressed_size, true);
#endif
	}

	return 0l;
}