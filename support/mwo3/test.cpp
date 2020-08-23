#include "MWo3.h"
#include <time.h>
#include <fileio.h>
#include <vector>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <ifstream>
#include <ofstream>
#include <string>
#include <vector>

using namespace std;

int defaultlevel = 7;

UCHAR* ReadAllFromFile(const char* name, ULONG& fileSize)
{

	std::ifstream file(name);
	
	file.seekg(0, file.end);
	int fsize = file.tellg();
	file.seekg(0, file.beg);
	
	std::vector<char> buffer(fsize);

	if (file.read(&buffer[0], fsize))
	{
		UCHAR* fileData = (UCHAR*)new char[fileSize];
		memcpy(fileData, &buffer[0], fileSize);
		return fileData;
	}

	return nullptr;
}

bool WriteToFile(const char* fileName, const char* src, int srcSize, int unknown, const char* fileAttribute)
{
	ofstream outputFile;
	outputFile.open(fileName);
	outputFile.write(src, srcSize);

	outputFile.close();

	return true;
}


void Encode(const string& name, int level)
{
	ULONG srcsize = 0;
	UCHAR* src = ReadAllFromFile(name.c_str(), srcsize);
	if (!src) return;
	level = (level >= 0 && level <= 9) ? level : defaultlevel;

	UCHAR* dst = new UCHAR[srcsize];
	ULONG dstsize = 0;
	//clock_t begin, end;
	//begin = clock();
	compress(src, srcsize, dst, dstsize, level);
	//deflate();
	//end = clock();

	//printf("Compress %s success. use %ld ms, level %d.\n", name, (end - begin), level);
	string outName(name, 0, name.find_last_of("."));
	outName += ".mwo";
	WriteToFile(outName.c_str(), (const char*)dst, dstsize, 0, "wb+");

	delete[] src;
	delete[] dst;
}
void Decode(const char* name)
{
	ULONG srcsize = 0;
	UCHAR* src = ReadAllFromFile(name, srcsize);
	if (!src) return;

	const UCHAR *p = src;
	int uncompresssize = 0;
	uncompresssize = getnum(p);

	int t = 0;
	t = getnum(p);

	int a1 = t >> 1;
	int v0 = a1 >> 1;
	v0 = v0 & 0xf;
	v0 = v0 + 0x8;
	v0 = 1 << v0;

	if (v0<uncompresssize || (a1 & 0x21) != 1) {
		if ((a1 & 0x40) != 0) {
			getnum(p);
		}
	}
	getnum(p);

	UCHAR* dst = new UCHAR[uncompresssize];
	uncompress(p, src + srcsize, dst, dst + uncompresssize);
	std::string outName = name;
	outName  += ".bin";
	if (WriteToFile(outName.c_str(), (const char*)dst, uncompresssize, 0, "wb+"))
		printf("Uncompress %s to %s success.\n", name, outName.c_str());

	delete[] src;
	delete[] dst;
}

void PrintHelp()
{
	printf("Error Argv.\n");
	printf("SRWZ.exe [-c|-d] infile [level]\n");
}

int main(int argc, char* argv[], char* envp[])
{
	int nRetCode = -1;
	if (argc < 3) { PrintHelp(); return nRetCode; }

	if (!stricmp(argv[1], "-d"))
	{
		Decode(std::string(argv[2]).c_str());
	}
	else if (!stricmp(argv[1], "-c"))
	{
		Encode(string(argv[2]), ((argc >= 4) ? atoi(argv[3]) : defaultlevel));
	}
	else
	{
		PrintHelp();
	}

    return 0;
}

