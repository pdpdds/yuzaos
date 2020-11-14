#include <vector>
#include <list>
#include <deque>
#include <map>
#include <functional>
#include <algorithm>

#include <time.h>
#include <math.h>

using namespace std;

void TestVector()
{
	vector<int> vec;
	vec.push_back(100);
	vec.push_back(105);
	vec.push_back(120);
	vec.push_back(140);

	while (vec.size() > 0)
	{
		int data = vec.back();
		printf("%d\n", data);
		vec.pop_back();
	}
}

typedef struct tag_A
{
	int data1;
	int data2;
}A;

void TestList()
{
	list<A> data;
	int counter = 0;

	for (int i = 0; i <= 10; ++i) 
	{
		A a;
		a.data1 = i;
		a.data2 = i + 1;
		data.push_back(a);
	}
	list<A>::iterator iter = data.begin();
	for (size_t i = 0; i < data.size(); i++, iter++)
	{
		printf("item 0x%d 0x%d done\n", (*iter).data1, (*iter).data2);
	}
}

void TestMap()
{
	map<int, int> mapData;
	map<int, int>::iterator iter;

	mapData[1] = 2;
	mapData[2] = 4;
	mapData[4] = 8;
	mapData[3] = 6;
	mapData[5] = 10;
	mapData[6] = 12;
	mapData[7] = 14;
	mapData[8] = 16;

	for (iter = mapData.begin(); iter != mapData.end(); iter++)
	{
		printf("%d  %d\n", (*iter).first, (*iter).second);
	}

	mapData.erase(6);

	for (iter = mapData.begin(); iter != mapData.end(); iter++)
	{
		printf("%d  %d\n", (*iter).first, (*iter).second);
	
	}
}

void TestString()
{
	std::string str("abc");
	printf("%s\n", str.c_str());

	std::string str2 = str;
	str2 = "cdf";
	printf("%s\n", str2.c_str());

	std::string a = "foo";
	std::string b = "bar";
	a = b;
	cout << a.c_str() << endl;
}

void TestDeque()
{
	std::deque<int> a;

	for (int i = 1; i < 6; i++)
		a.push_front(i);

	for (int i = 0; i < 5; i++)
		printf("%d\n", a[i]);
}

void TestAlogirithm()
{
	int data[5] = { 100, 30, 25, 43, 1};
	sort(data, data + 5);

	for (int i = 0; i < 5; i++)
		printf("%d\n", data[i]);

	vector<int> v;
	srand((int)time(NULL)); 
	
	for (int i = 0; i < 5; i++) 
	{ 
		v.push_back(rand() % 10);
	}

	sort(v.begin(), v.end(), greater<int>());

	for (vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
	{
		printf("[%d]\n", *iter);
	}
}

int main(int argc, char** argv)
{
	TestVector();
	TestList();
	TestMap();
	TestString();
	TestDeque();
	TestAlogirithm();

	return 0;
}

