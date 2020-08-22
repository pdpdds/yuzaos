/*
 * Realmode Emulator Plugin
 * - By John Hodge (thePowersGang)
 *
 * This code is published under the FreeBSD licence
 * (See the file COPYING for details)
 *
 * ---
 * Sample Emulator Configuration Include
 */
#ifndef _RME_CONFIG_H_
#define _RME_CONFIG_H_

#define DEBUG	0	// Enable debug? (2 enables a register dump)
//#define ERR_OUTPUT	1	// Enable using printf on an error?

#include "windef.h"
#include <stdio.h>	// printf
//#include <stdlib.h>	// calloc
#include <stdint.h>

/*extern uint8_t	inb(uint16_t port);
extern uint16_t	inw(uint16_t port);
extern uint32_t	inl(uint16_t port);
extern void	outb(uint16_t port, uint8_t val);
extern void	outw(uint16_t port, uint16_t val);
extern void	outl(uint16_t port, uint32_t val);*/


extern int _outp(unsigned short, int);
extern unsigned long _outpd(unsigned int, int);
extern unsigned short _outpw(unsigned short, unsigned short);
extern int _inp(unsigned short);
extern unsigned short _inpw(unsigned short);
extern unsigned long _inpd(unsigned int shor);

#endif
