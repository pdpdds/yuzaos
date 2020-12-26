#include "Example.h"
#include "CircularBuffer.h"

CircularBuffer<int, 400> buffer;

unsigned long time = 0;

void Example()
{



	int reading = 0;
	float avg = 0.0;
	buffer.push(reading);


	using index_t = decltype(buffer)::index_t;
	for (index_t i = 0; i < buffer.size(); i++) {
		avg += buffer[i] / buffer.size();
	}

}
