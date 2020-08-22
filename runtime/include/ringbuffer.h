#pragma once

/* Includes 
 * - Library */
#include <osdefs.h>
#include <_spinlock.h>

typedef struct _RingBuffer
{
	/* The buffer */
	uint8_t *Buffer;

	/* Length of buffer */
	size_t Length;

	/* Index */
	int IndexWrite;
	int IndexRead;

	/* Lock */
	Spinlock_t Lock;

} RingBuffer_t;

/* Initialise a new ring buffer */
CRTDECL(RingBuffer_t*, RingBufferCreate(size_t Size));

/* Construct a new ring buffer */
CRTDECL(void, RingBufferConstruct(RingBuffer_t *RingBuffer, uint8_t *Buffer, size_t BufferLength));

/* Destroy Ringbuffer */
CRTDECL(void, RingBufferDestroy(RingBuffer_t *RingBuffer));

/* Write to buffer */
CRTDECL(int, RingBufferWrite(RingBuffer_t *RingBuffer, size_t SrcLength, uint8_t *Source));

/* Read from buffer */
CRTDECL(int, RingBufferRead(RingBuffer_t *RingBuffer, size_t DestLength, uint8_t *Destination));

/* How many bytes are available in buffer to be read */
CRTDECL(size_t, RingBufferSize(RingBuffer_t *RingBuffer));

/* How many bytes are ready for usage */
CRTDECL(int, RingBufferSpaceAvailable(RingBuffer_t *RingBuffer));

CRTDECL(size_t, RingBufferUnread(RingBuffer_t *RingBuffer));

