#include <stdio.h>
#include <uuid.h>

int main(int argc, char* argv[])
{
    uuid_t uuid;

    //고유식별자 생성
    uuid_generate_time_safe(uuid);

    // uuid를 소문자로 구성된 문자열로 얻는다.
    char uuid_str[37];    
    uuid_unparse_lower(uuid, uuid_str);
    printf("UUID = %s\n", uuid_str);

    return 0;
}