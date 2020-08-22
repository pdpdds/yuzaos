/*
 * Realmode Emulator
 * - IO Operations Header
 * 
 */
#ifndef _RME_OPS_IO_H_
#define _RME_OPS_IO_H_

#define	outB(state,port,val)	(_outp(port,val),0)	// Write 1 byte to an IO Port
#define	outW(state,port,val)	(_outpw(port,val),0)	// Write 2 bytes to an IO Port
#define	outD(state,port,val)	(_outpd(port,val),0)	// Write 4 bytes to an IO Port
#define	inB(state,port,dst)	(*(dst)=_inp((port)),0)	// Read 1 byte from an IO Port
#define	inW(state,port,dst)	(*(dst)=_inpw((port)),0)	// Read 2 bytes from an IO Port
#define	inD(state,port,dst)	(*(dst)=_inpd((port)),0)	// Read 4 bytes from an IO Port

#endif
