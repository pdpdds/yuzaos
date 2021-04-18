#ifndef AUDIO_FW_H
#define AUDIO_FW_H

//#include <minix/drivers.h>
//#include <minix/chardriver.h>
#include "ioc_sound.h"

#define u32_t unsigned int
#define endpoint_t unsigned int
#define cp_grant_id_t unsigned int
#define phys_bytes unsigned int

/*===========================================================================*
 *                Messages for CHARACTER device drivers			     *
 *===========================================================================*/

 /* Message types for character device drivers. */
#define DEV_RQ_BASE   0x400	/* base for character device request types */
#define DEV_RS_BASE   0x500	/* base for character device response types */

#define CANCEL       	(DEV_RQ_BASE +  0) /* force a task to cancel */
#define DEV_OPEN     	(DEV_RQ_BASE +  6) /* open a minor device */
#define DEV_CLOSE    	(DEV_RQ_BASE +  7) /* close a minor device */
#define DEV_SELECT	(DEV_RQ_BASE + 12) /* request select() attention */
#define DEV_STATUS   	(DEV_RQ_BASE + 13) /* request driver status */
#define DEV_REOPEN     	(DEV_RQ_BASE + 14) /* reopen a minor device */

#define DEV_READ_S	(DEV_RQ_BASE + 20) /* (safecopy) read from minor */
#define DEV_WRITE_S   	(DEV_RQ_BASE + 21) /* (safecopy) write to minor */
#define DEV_SCATTER_S  	(DEV_RQ_BASE + 22) /* (safecopy) write from a vector */
#define DEV_GATHER_S   	(DEV_RQ_BASE + 23) /* (safecopy) read into a vector */
#define DEV_IOCTL_S    	(DEV_RQ_BASE + 24) /* (safecopy) I/O control code */


int drv_init(void);
int drv_init_hw(void);
int drv_reset(void);
int drv_start(int sub_dev, int DmaMode);
int drv_stop(int sub_dev);
int drv_set_dma(u32_t dma, u32_t length, int chan);
int drv_reenable_int(int chan);
int drv_int_sum(void);
int drv_int(int sub_dev);
int drv_pause(int chan);
int drv_resume(int chan);
int drv_io_ctl(int request, void * val, int * len, int sub_dev);
int drv_get_irq(char *irq);
int drv_get_frag_size(u32_t *frag_size, int sub_dev);

#define EXTERN extern 



/* runtime status fields */
typedef struct {
	int readable;
	int writable;
	int DmaSize;
	int NrOfDmaFragments;
	int MinFragmentSize;
	int NrOfExtraBuffers;
	int Nr;                                   /* sub device number */
	int Opened;                               /* sub device opened */
	int DmaBusy;                              /* is dma busy? */
	int DmaMode;                              /* DEV_WRITE / DEV_READ */
	int DmaReadNext;                          /* current dma buffer */
	int DmaFillNext;                          /* next dma buffer to fill */
	int DmaLength;
	int BufReadNext;                          /* start of extra circular buffer */
	int BufFillNext;                          /* end of extra circular buffer */
	int BufLength;
	int RevivePending;                        /* process waiting for this dev? */
	int ReviveStatus;                         /* return val when proc unblocked */
	endpoint_t ReviveProcNr;                  /* the process to unblock */
	cp_grant_id_t ReviveGrant;		  /* grant id associated with io */
	void *UserBuf;                            /* address of user's data buffer */
	int ReadyToRevive;                        /* are we ready to revive process?*/
	endpoint_t SourceProcNr;                  /* process to send notify to (FS) */
	u32_t FragSize;                           /* dma fragment size */
	char *DmaBuf;        /* the dma buffer; extra space for 
												  page alignment */
	phys_bytes DmaPhys;                       /* physical address of dma buffer */
	char* DmaPtr;                             /* pointer to aligned dma buffer */
	int OutOfData;                            /* all buffers empty? */
	char *ExtraBuf;                           /* don't use extra buffer;just 
											   declare a pointer to supress
											   error messages */
} sub_dev_t;

typedef struct {
	int minor_dev_nr;
	int read_chan;
	int write_chan;
	int io_ctl;
} special_file_t;

typedef struct {
	char* DriverName;
	int NrOfSubDevices;
	int NrOfSpecialFiles;
} drv_t;

EXTERN drv_t drv;
EXTERN sub_dev_t sub_dev[];
EXTERN special_file_t special_file[];

/* Number of bytes you can DMA before hitting a 64K boundary: */
#define dma_bytes_left(phys)    \
   ((unsigned) (sizeof(int) == 2 ? 0 : 0x10000) - (unsigned) ((phys) & 0xFFFF))

#define NO_CHANNEL -1

#define TRUE 1
#define FALSE 0
#define NO_DMA 0

#define OK TRUE
#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif

#endif /* AUDIO_FW_H */
