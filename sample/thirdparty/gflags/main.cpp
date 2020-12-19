#include <gflags.h>
#include <iostream>

using namespace std;

DEFINE_int32(port, 0, "What port to listen on");
DEFINE_string(languages, "english", "comma-separated list of languages to offer in the 'lang' menu");

//포트 값이 유효한지를 검사하는 콜백 함수
static bool ValidatePort(const char* flagname, gflags::int32 value) {
	if (value > 0 && value < 32768)   // value is ok
		return true;
	printf("Invalid port value for --%s: %d\n", flagname, (int)value);
	return false;
}

//언어가 프로그램에서 지원하는지를 검사하는 콜백 함수
static bool ValidateLanguage(const char* flagname, const std::string& value) {

	if (value.compare("english") == 0 ||
		value.compare("french") == 0 ||
		value.compare("german") == 0)
		return true;

	return false;
}

int main(int argc, char* argv[])
{
	//포트값이 유효한지를 검사하는 콜백 함수 추가
	bool bResult = gflags::RegisterFlagValidator(&FLAGS_port, &ValidatePort);
	assert(bResult == true);

	//언어 스트링이 유효한지를 체크하는 콜백 함수 추가
	bResult = gflags::RegisterFlagValidator(&FLAGS_languages, &ValidateLanguage);
	assert(bResult == true);
	//커맨드 라인을 파싱
	gflags::ParseCommandLineFlags(&argc, &argv, true);

	cout << FLAGS_languages.c_str() << endl;
	cout << FLAGS_port << endl;
	return 0;
}
