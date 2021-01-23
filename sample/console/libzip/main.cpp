#include <vector>
#include <fstream>

#include "libzippp.h"


using namespace libzippp;
using namespace std;

int main(int argc, char* argv[])
{
	{
		ZipArchive zf("archive.zip");
		zf.open(ZipArchive::ReadOnly);

		vector<ZipEntry> entries = zf.getEntries();
		vector<ZipEntry>::iterator it;
		for (it = entries.begin(); it != entries.end(); ++it) {
			ZipEntry entry = *it;
			string name = entry.getName();
			int size = entry.getSize();

			//the length of binaryData will be size
			void* binaryData = entry.readAsBinary();

			//the length of textData will be size
			string textData = entry.readAsText();

			//...
		}

		zf.close();
	}


	{
		ZipArchive zf("archive.zip");
		zf.open(ZipArchive::ReadOnly);

		//raw access
		char* data = (char*)zf.readEntry("myFile.txt", true);
		ZipEntry entry1 = zf.getEntry("myFile.txt");
		string str1(data, entry1.getSize());

		//text access
		ZipEntry entry2 = zf.getEntry("myFile.txt");
		string str2 = entry2.readAsText();

		zf.close();
	}

	{
		ZipArchive zf("archive.zip");
		zf.open(ZipArchive::ReadOnly);

		ZipEntry largeEntry = zf.getEntry("largeentry");
		std::ofstream ofUnzippedFile("largeFileContent.data");
		largeEntry.readContent(ofUnzippedFile);
		ofUnzippedFile.close();

		zf.close();
	}

	{
		ZipArchive zf("archive.zip");
		zf.open(ZipArchive::Write);
		zf.addEntry("folder/subdir/");

		const char* textData = "Hello,World!";
		zf.addData("helloworld.txt", textData, 12);

		zf.close();
	}

	{
		ZipArchive zf("archive.zip");
		zf.open(ZipArchive::Write);
		zf.deleteEntry("myFile.txt");
		zf.deleteEntry("myDir/subDir/");
		zf.close();
	}

	{
		/*char* buffer = someData;
		uint32_t bufferSize = sizeOfBuffer;

		ZipArchive* zf = ZipArchive::fromBuffer(buffer, bufferSize);
		...
		zf->close();
		delete zf;*/
	}

	
}
