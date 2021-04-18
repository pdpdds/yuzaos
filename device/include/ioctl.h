#pragma once
#include <../include/types.h>

/* Field names for SYS_DEVIO, SYS_VDEVIO, SYS_SDEVIO. */
#define DIO_REQUEST	m2_i3	/* device in or output */
#   define _DIO_INPUT		0x001
#   define _DIO_OUTPUT		0x002
#   define _DIO_DIRMASK		0x00f
#   define _DIO_BYTE		0x010
#   define _DIO_WORD		0x020
#   define _DIO_LONG		0x030
#   define _DIO_TYPEMASK	0x0f0
#   define _DIO_SAFE		0x100
#   define _DIO_SAFEMASK	0xf00
#   define DIO_INPUT_BYTE	    (_DIO_INPUT|_DIO_BYTE)
#   define DIO_INPUT_WORD	    (_DIO_INPUT|_DIO_WORD)
#   define DIO_INPUT_LONG	    (_DIO_INPUT|_DIO_LONG)
#   define DIO_OUTPUT_BYTE	    (_DIO_OUTPUT|_DIO_BYTE)
#   define DIO_OUTPUT_WORD	    (_DIO_OUTPUT|_DIO_WORD)
#   define DIO_OUTPUT_LONG	    (_DIO_OUTPUT|_DIO_LONG)
#   define DIO_SAFE_INPUT_BYTE      (_DIO_INPUT|_DIO_BYTE|_DIO_SAFE)
#   define DIO_SAFE_INPUT_WORD      (_DIO_INPUT|_DIO_WORD|_DIO_SAFE)
#   define DIO_SAFE_INPUT_LONG      (_DIO_INPUT|_DIO_LONG|_DIO_SAFE)
#   define DIO_SAFE_OUTPUT_BYTE     (_DIO_OUTPUT|_DIO_BYTE|_DIO_SAFE)
#   define DIO_SAFE_OUTPUT_WORD     (_DIO_OUTPUT|_DIO_WORD|_DIO_SAFE)
#   define DIO_SAFE_OUTPUT_LONG     (_DIO_OUTPUT|_DIO_LONG|_DIO_SAFE)
#define DIO_PORT	m2_l1	/* single port address */
#define DIO_VALUE	m2_l2	/* single I/O value */
#define DIO_VEC_ADDR	m2_p1   /* address of buffer or (p,v)-pairs */
#define DIO_VEC_SIZE	m2_l2   /* number of elements in vector */
#define DIO_VEC_ENDPT	m2_i2   /* number of process where vector is */
#define DIO_OFFSET	m2_i1	/* offset from grant */

/* Ioctls have the command encoded in the low-order word, and the size
 * of the parameter in the high-order word. The 3 high bits of the high-
 * order word are used to encode the in/out/void status of the parameter.
 */
#define _IOCPARM_MASK		0x0FFF
#define _IOCPARM_MASK_BIG	0x0FFFFF
#define _IOC_VOID		0x20000000
#define _IOCTYPE_MASK		0xFFFF
#define _IOC_IN			0x40000000
#define _IOC_OUT		0x80000000
#define _IOC_INOUT		(_IOC_IN | _IOC_OUT)

 /* Flag indicating ioctl format with only one type field, and more bits
  * for the size field (using mask _IOCPARM_MASK_BIG).
  */
#define _IOC_BIG		0x10000000

#define _IO(x,y)	((x << 8) | y | _IOC_VOID)
#define _IOR(x,y,t)	((x << 8) | y | ((sizeof(t) & _IOCPARM_MASK) << 16) |\
				_IOC_OUT)
#define _IOW(x,y,t)	((x << 8) | y | ((sizeof(t) & _IOCPARM_MASK) << 16) |\
				_IOC_IN)
#define _IORW(x,y,t)	((x << 8) | y | ((sizeof(t) & _IOCPARM_MASK) << 16) |\
				_IOC_INOUT)

#define _IOW_BIG(y,t)  (y | ((sizeof(t) & _IOCPARM_MASK_BIG) << 8) \
	| _IOC_IN | _IOC_BIG)
#define _IOR_BIG(y,t)  (y | ((sizeof(t) & _IOCPARM_MASK_BIG) << 8) \
	| _IOC_OUT | _IOC_BIG)
#define _IORW_BIG(y,t) (y | ((sizeof(t) & _IOCPARM_MASK_BIG) << 8) \
	| _IOC_INOUT | _IOC_BIG)

  /* Decode an ioctl call. */
#define _MINIX_IOCTL_SIZE(i)		(((i) >> 16) & _IOCPARM_MASK)
#define _MINIX_IOCTL_IOR(i)		((i) & _IOC_OUT)
#define _MINIX_IOCTL_IORW(i)		((i) & _IOC_INOUT)
#define _MINIX_IOCTL_IOW(i)		((i) & _IOC_IN)

/* Recognize and decode size of a 'big' ioctl call. */
#define _MINIX_IOCTL_BIG(i) 		((i) & _IOC_BIG)
#define _MINIX_IOCTL_SIZE_BIG(i)	(((i) >> 8) & _IOCPARM_MASK_BIG)

#define sys_voutb(out, n) at_voutb((out), (n))
int at_voutb(pvb_pair_t*, int n);
#define sys_vinb(in, n) at_vinb((in), (n))
int at_vinb(pvb_pair_t*, int n);

int at_out(int line, u32_t port, u32_t value, char* typename,
	int type);
int at_in(int line, u32_t port, u32_t* value, char* typename,
	int type);

#define sys_outb(p, v) at_out(__LINE__, (p), (v), "outb", _DIO_BYTE)
#define sys_inb(p, v) at_in(__LINE__, (p), (v), "inb", _DIO_BYTE)
#define sys_outl(p, v) at_out(__LINE__, (p), (v), "outl", _DIO_LONG)

#ifdef __cplusplus
extern "C" {
#endif
	int _outp(unsigned short, int);
	unsigned long _outpd(unsigned int, int);
	unsigned short _outpw(unsigned short, unsigned short);
	int _inp(unsigned short);
	unsigned short _inpw(unsigned short);
	unsigned long _inpd(unsigned int shor);

	void OutPortByte(unsigned short port, unsigned char value);
	void OutPortWord(unsigned short port, unsigned short value);
	void OutPortDWord(unsigned short port, unsigned int value);
	long InPortDWord(unsigned int port);
	unsigned char InPortByte(unsigned short port);
	unsigned char InPortWord(unsigned short port);
	unsigned char inports(unsigned short port, unsigned short* buffer, int count);

#ifdef __cplusplus
}
#endif