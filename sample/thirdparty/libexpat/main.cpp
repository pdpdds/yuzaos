#include "expat.h"
#include "ExpatImpl.h"
#include <minwindef.h>
#include <stdio.h>
//샘플 파서 선언
class CMyXML : public CExpatImpl <CMyXML>
{
public:
	//중략
	// Start element handler
	void OnStartElement(const XML_Char* pszName, const XML_Char** papszAttrs)
	{
		printf("We got a start element %s\n", pszName);
		return;
	}

	// End element handler
	void OnEndElement(const XML_Char* pszName)
	{
		printf("We got an end element %s\n", pszName);
		return;
	}

	// Character data handler
	void OnCharacterData(const XML_Char* pszData, int nLength)
	{
		// note, pszData is NOT null terminated
		printf("We got %d bytes of data\n", nLength);
		return;
	}
};
//XML 파일을 파싱한다.
bool ParseSomeXML(LPCSTR pszFileName)
{
	//파서 선언
	CMyXML sParser;
	if (!sParser.Create())
		return false;
	//XML 파일을 연다.
	FILE* fp = NULL;
	fp = fopen(pszFileName, "r");
	if (fp == 0)
		return false;

	//한 줄씩 파일을 읽어들이며 데이터를 처리한다.
	bool fSuccess = true;
	while (!feof(fp) && fSuccess)
	{
		LPSTR pszBuffer = (LPSTR)sParser.GetBuffer(256); // REQUEST
		if (pszBuffer == NULL)
			fSuccess = false;
		else
		{
			int nLength = fread(pszBuffer, 1, 256, fp); // READ
			//버퍼를 파싱하고 파싱에 성공하면 파서의 핸들러가 호출된다.
			fSuccess = sParser.ParseBuffer(nLength, nLength == 0); // PARSE
		}
	}
	
	return true;
}

int main(int argc, char* argv[])
{
	ParseSomeXML("example.xml");
	return 0;
}
