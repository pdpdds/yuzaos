#include <stdint.h>
#include <systemcall_impl.h>

#define OK 0

typedef uint8_t 	u8_t;
typedef uint16_t	u16_t;
typedef uint32_t	u32_t;
typedef uint64_t 	u64_t;

typedef int8_t		i8_t;
typedef int16_t		i16_t;
typedef int32_t		i32_t;
typedef int64_t		i64_t;

typedef struct { u16_t port;  u8_t value; } pvb_pair_t;
typedef struct { u16_t port; u16_t value; } pvw_pair_t;
typedef struct { u16_t port; u32_t value; } pvl_pair_t;

/*int _outp(unsigned short, int);
unsigned long _outpd(unsigned int, int);
unsigned short _outpw(unsigned short, unsigned short);
int _inp(unsigned short);
unsigned short _inpw(unsigned short);
unsigned long _inpd(unsigned int shor);

void OutPortByte(unsigned short port, unsigned char value)
{
	_outp(port, value);
}

void OutPortWord(unsigned short port, unsigned short value)
{
	_outpw(port, value);
}

void OutPortDWord(unsigned short port, unsigned int value)
{
	_outpd(port, value);
}

long InPortDWord(unsigned int port)
{
	return _inpd(port);
}

unsigned char InPortByte(unsigned short port)
{

	return (unsigned char)_inp(port);
}

unsigned char InPortWord(unsigned short port)
{
	return _inpw(port);
}*/

unsigned char inports(unsigned short port, unsigned short* buffer, int count)
{
	__asm
	{
		mov dx, port
		mov edi, buffer; Address of buffer
		mov ecx, count; Repeat count times
		rep insw
	}
}

int at_vinb(pvb_pair_t* pvb, int n)
{
	return 0;
	/*int s, i;
	if ((s = sys_vinb(pvb, n)) == OK)
		return OK;
	//printf("at_wini%ld: sys_vinb failed: %d pvb (%d):\n", w_instance, s, n);
	for (i = 0; i < n; i++)
		printf("%2d: %4x\n", i, pvb[i].port);
	Syscall_Panic("sys_vinb failed");*/
}

int at_out(int line, u32_t port, u32_t value, char* typename, int type)
{
	return 0;
	/*int s;
	s = sys_out(port, value, type);
	if (s == OK)
		return OK;
	//printf("at_wini%ld: line %d: %s failed: %d; %x -> %x\n",
	//	w_instance, line, typename, s, value, port);
	Syscall_Panic("sys_out failed");*/
}

int at_in(int line, u32_t port, u32_t* value, char* typename, int type)
{
	return 0;
	/*int s;
	s = sys_in(port, value, type);
	if (s == OK)
		return OK;
	//printf("at_wini%ld: line %d: %s failed: %d; port %x\n",
		//w_instance, line, typename, s, port);
	Syscall_Panic("sys_in failed");*/
}

int at_voutb(pvb_pair_t* pvb, int n)
{
	return 0;
	/*int s, i;
	if ((s = sys_voutb(pvb, n)) == OK)
		return OK;
	printf("at_wini%ld: sys_voutb failed: %d pvb (%d):\n", w_instance, s, n);
	for (i = 0; i < n; i++)
		printf("%2d: %4x -> %4x\n", i, pvb[i].value, pvb[i].port);
	panic("sys_voutb failed");*/
}
