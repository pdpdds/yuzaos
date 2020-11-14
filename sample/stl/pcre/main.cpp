#include <windef.h>
#include "stdio.h"
#include "string.h"
#include <memory.h>
#include <pcre2.h>
#include <string>
#include "jpcre2.hpp"

typedef jpcre2::select<char> jp;

bool is_email_valid(const std::string& email)
{
    jp::Regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+)+)");
   
    return pattern.match(email);
}

bool is_email_valid2(const std::string& email)
{
    jp::Regex pattern("[0-9a-zA-Z]([-_.]?[0-9a-zA-Z])*@[0-9a-zA-Z]([-_.]?[0-9a-zA-Z])*[.][a-zA-Z]{2,3}");
    return pattern.match(email);
}

int main() 
{
    std::vector<std::string> emails;
    emails.push_back("yuza@daum.net");
    emails.push_back("yuza3@daum.net");
    emails.push_back("orange@daum.net");
    emails.push_back("orange");
    emails.push_back("skyos32@");
    emails.push_back("skyos32@naver.coma.a");
    emails.push_back("s-a@daum.net");
    emails.push_back("s.@daum.net");
   
    printf("regex email pattern 1\n");
    for (const auto& email : emails)
    {
        if (is_email_valid(email))
            printf("%s is valid address\n", email.c_str());
    }

    printf("regex email pattern 2\n");
    for (const auto& email : emails)
    {
        if (is_email_valid2(email))
            printf("%s is valid address\n", email.c_str());
    }

    return 0;
}

void test()
{
	jp::Regex re("\\w+ect");

	if (re.match("I am the subject")) //always uses a new temporary match object
		std::cout << "matched (case sensitive)";
	else
		std::cout << "Didn't match";

	//For case insensitive match, re-compile with modifier 'i'
	re.addModifier("i").compile();

	if (re.match("I am the subjEct")) //always uses a new temporary match object
		std::cout << "matched (case insensitive)";
	else
		std::cout << "Didn't match";

	size_t count = jp::Regex("[a]", "i").match("I am the subject", "g"); //always uses a new temporary match object
}