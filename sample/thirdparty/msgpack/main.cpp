#include <msgpack.h>
#include <stdio.h>

int main(void)
{
    //msgpack을 위한 버퍼 선언, 초기화
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    char szYuza[] = "YUZA OS";

    //직렬화를 위한 패커 준비
    //msgpack_sbuffer_write 함수는 콜백함수다.
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    //데이터 패킹. 5개를 패킹한다.
    msgpack_pack_array(&pk, 5);
    msgpack_pack_int(&pk, 12345); //int
    msgpack_pack_true(&pk); //bool
    msgpack_pack_str(&pk, strlen(szYuza)); //문자열
    msgpack_pack_str_body(&pk, szYuza, strlen(szYuza));
    msgpack_pack_short(&pk, 12); // short
    msgpack_pack_char(&pk, 'a'); // char

    //역직렬화 테스트
    msgpack_zone mempool;
    msgpack_zone_init(&mempool, 2048);

    msgpack_object deserialized;
    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);

    //결과 출력
    msgpack_object_print(stdout, deserialized);
    printf("\n");

    msgpack_zone_destroy(&mempool);
    msgpack_sbuffer_destroy(&sbuf);

    return 0;
}