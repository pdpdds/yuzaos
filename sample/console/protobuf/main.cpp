#include "stdio.h"
#include "string.h"
#include <memory.h>
#include <SystemCall_Impl.h>
#include <GUIConsoleFramework.h>

#include "test.pb-c.h"

int main_impl(int argc, char** argv)
{
	YUZA__Person person = YUZA__PERSON__INIT;
	person.name = "Juhang Park";
	person.id = 33;
	person.n_phone = 1;
	YUZA__Person__PhoneNumber* phone_list[1];
	person.phone = phone_list;
	
	YUZA__Person__PhoneNumber phone = YUZA__PERSON__PHONE_NUMBER__INIT;
	phone.number = "010-0000-0000";
	phone.type = YUZA__PERSON__PHONE_TYPE__WORK;
	phone_list[0] = &phone;

	YUZA__Person__PhoneNumber__Comment comment = YUZA__PERSON__PHONE_NUMBER__COMMENT__INIT;
	comment.comment = "yuzaos guy";
	phone.comment = &comment;

	size_t size, packed_size;
	unsigned char* packed;

	size = yuza__person__get_packed_size(&person);
	packed = (unsigned char*)malloc(size);
	assert(packed);

	packed_size = yuza__person__pack(&person, packed);
	assert(size == packed_size);
	
	YUZA__Person* person2;
	person2 = yuza__person__unpack(NULL, size, packed);
	yuza__person__free_unpacked(person2, NULL);
	free(packed);

	printf("test succeeded.\n");

	return 0;
}

int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);

	return 0;
}

