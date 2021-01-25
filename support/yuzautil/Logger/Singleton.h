/** 
*  @file		NFSingleton.h
*  @brief		싱글톤 클래스
*  @remarks	
*  @author		강동명(edith2580@gmail.com)
*  @date		2009-04-02
*/

#pragma once

#include <minwindef.h>
#include <stdio.h>
#include <assert.h>

/*
#include "NFSingleton.h"

class Test : public Nave::NFSingleton<Test>
{
public:
Test()
{
printf("Test::Test()\n");
}
~Test()
{
printf("Test::~Test()\n");
}

int GetType()
{
return 0;
}
};

INT _tmain(INT argc, WCHAR* argv[])
{
// 만약 싱글톤 객체를 사용하려면 
// 아래와 같이 사용하기 위해서 한번 New를 하고
Test* pTest = new Test;


// 다른 위치의 다른함수.
Test::GetInstance().GetType(); 와 같이 사용하면 된다.


delete pTest;
}
*/

#pragma warning (disable : 4311)
#pragma warning (disable : 4312)
#pragma warning (disable : 6011)

/** 
*  @class        NFSingleton
*  @brief        클래스 객체를 싱글톤으로 생성합니다.
*  @remarks      
*             
*  @warning	  해당 싱글톤 객체는 처음 생성은 직접 new로 생성을 해주고	\r\n
*				  delete가 될때까지 싱글톤 같이 사용하며 프로그램이 종료될때\r\n
*				  delete를 해주면 된다.										
*  @par          
*  @author  Edith
*  @date    2009-04-05
*/
template <typename T> class CSingleton
{

public:
	/// NFSingleton 생성자
	CSingleton (void)
	{
		assert( !s_pSingleton && "NFSingleton : You can't create more"
			" object, because this object is SINGLETON.");

#if defined( _MSC_VER ) && _MSC_VER < 1200	 
		int iOffset = (int)(T*)1 - (int)(CSingleton <T>*)(T*)1;
		s_pSingleton = (T*)((int) this + iOffset);
#else
		s_pSingleton = static_cast< T* >( this );
#endif
	}

	/// NFSingleton 소멸자
	~CSingleton (void)
	{
		assert( s_pSingleton && "NFSingleton : This object may be "
			"destroyed by someone's memory control or other causes.");

		s_pSingleton = 0;
	}

	/// 해당 싱글톤 객체가 생성되는지 확인한다.
	static bool IsAlive()
	{
		return s_pSingleton ? true : false;
	}

	/// 싱글톤 객체의 인스턴스를 얻는다.
	static T& GetInstance(void)
	{
		assert (s_pSingleton && "NFSingleton : Something is wrong."
			" This object destroyed by something bad method or"
			"you didn't create yet!");

		return *s_pSingleton;
	}

	/// 싱글톤 객체의 유일한 인스턴스의 포인터를 얻는다
	static T* GetInstancePtr(void)
	{
		assert (s_pSingleton && "NFSingleton : Something is wrong."
			" This object destroyed by something bad method.");

		return s_pSingleton;
	}

private:
	/// 싱글톤 객체를 담을 객체의 포인터
	static T* s_pSingleton;
};

/// 싱글톤 객체 변수를 초기화 한다.
template<typename T> T* CSingleton<T>::s_pSingleton = 0;

