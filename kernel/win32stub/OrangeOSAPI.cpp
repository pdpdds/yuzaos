#include "stdafx.h"
#include <windows.h> 
//#include "orangeos.h"
//#include "stringdef.h"
#include "OrangeOSAPI.h"

typedef struct tag_SKY_APIStruct
{
	char* strAPIName;		//함수 이름
	int _APIIndex;
	void* ptrAPIFunction;		//함수 포인터
	tag_SKY_APIStruct * Next;
}SKY_APIStruct;
SKY_APIStruct* g_pRegisteredSkyAPIEntries = nullptr;

void RegisterOrangeOSAPI(int APIIndex, char* strAPIName, void * ptrAPIFunction)
{
	SKY_APIStruct* newAPIStruct;
	char *strName = nullptr;

	//printf("Register API : %s %x\n", strAPIName, (unsigned int)ptrAPIFunction);

	newAPIStruct = new SKY_APIStruct;
	strName = new char[strlen(strAPIName) + 1];

	strcpy(strName, strAPIName);

	//함수이름과 함수 포인터를 설정
	newAPIStruct->_APIIndex = APIIndex;
	newAPIStruct->strAPIName = strName;
	newAPIStruct->ptrAPIFunction = ptrAPIFunction;
	newAPIStruct->Next = nullptr;

	if (g_pRegisteredSkyAPIEntries) //루트 엔트리가 존재하면 검색을 해서 마지막에 새 엔트리를 연결하고 그렇지 않으면 새 엔트리를 루트 엔트리로
	{
		SKY_APIStruct* curAPIStruct = g_pRegisteredSkyAPIEntries;
		while (curAPIStruct->Next)
		{
			if (strcmp(curAPIStruct->strAPIName, strAPIName) == 0) //already the function exists update the pointer value
				break;
			curAPIStruct = curAPIStruct->Next;
		}
		curAPIStruct->Next = newAPIStruct;
	}
	else
		g_pRegisteredSkyAPIEntries = newAPIStruct;
}

void* GetOrangeOSAPI(char * strAPIName)
{
	SKY_APIStruct* curAPIStruct = g_pRegisteredSkyAPIEntries;
	while (curAPIStruct)
	{
		printf("[%s] [%s]\n", strAPIName, curAPIStruct->strAPIName);

		if (strcmp(curAPIStruct->strAPIName, strAPIName) == 0)
			return curAPIStruct->ptrAPIFunction;

		curAPIStruct = curAPIStruct->Next;
	}

	return 0;
}

void* GetOrangeOSAPIByIndex(int index)
{
	SKY_APIStruct* curAPIStruct = g_pRegisteredSkyAPIEntries;
	while (curAPIStruct)
	{
		
		if (curAPIStruct->_APIIndex == index)
			return curAPIStruct->ptrAPIFunction;

		curAPIStruct = curAPIStruct->Next;
	}

	return 0;
}