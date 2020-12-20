#include <stdio.h>
#include <memory.h>
#include <mpack.h>
#include <assert.h>

bool mpack_write(char** data, size_t& size)
{
	mpack_writer_t writer;
	mpack_writer_init_growable(&writer, data, &size);

	// write the example on the msgpack homepage
	//mpack_start_map(&writer, 2);
	mpack_write_cstr(&writer, "compact");
	mpack_write_bool(&writer, true);
	mpack_write_cstr(&writer, "schema");
	mpack_write_uint(&writer, 0);
	//mpack_finish_map(&writer);

	// finish writing
	if (mpack_writer_destroy(&writer) != mpack_ok) {
		printf("An error occurred encoding the data!\n");
		return false;
	}

	return true;
}

bool mpack_read(char* data, size_t size)
{
	mpack_reader_t reader;
	mpack_reader_init_data(&reader, data, size);


	mpack_tag_t tag = mpack_read_tag(&reader);


	if (mpack_tag_type(&tag) == mpack_type_str) {
		uint32_t length = mpack_tag_str_length(&tag);
		char buffer[128];

		while (length > 0) {
			size_t step = (length < sizeof(buffer)) ? length : sizeof(buffer);
			mpack_read_bytes(&reader, buffer, sizeof(buffer));
			if (mpack_reader_error(&reader) != mpack_ok) // critical check!
				break;
		}

		mpack_done_str(&reader);
	}

	tag = mpack_read_tag(&reader);

	if (mpack_tag_type(&tag) != mpack_type_bool) {
		printf("not a bool!");
		return false;
	}
	bool value = mpack_tag_bool_value(&tag);

	return true;
}



int main(int argc, char** argv)
{
	printf("%smpack test!!\n", (char*)argv[0]);

	char* data;
	size_t size;
	bool result = mpack_write(&data, size);
	assert(result == true);

	result = mpack_read(data, size);
	assert(result == true);


	printf("comlete!!\n");
	free(data);

	return 0;
}