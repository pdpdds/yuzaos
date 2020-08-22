#ifndef DMA_H
#define DMA_H

#include "windef.h"
#include <stdint.h>
// register ports
#define DMA_MASKCHANNEL_MASTER  0x0A
#define DMA_TRANSFERMODE_MASTER 0x0B
#define DMA_FLIPFLOP_MASTER     0x0C
#define DMA_RESET_MASTER        0x0D

#define DMA_MASKCHANNEL_SLAVE   0xD4
#define DMA_TRANSFERMODE_SLAVE  0xD6
#define DMA_FLIPFLOP_SLAVE      0xD8
#define DMA_RESET_SLAVE         0xDA

// mask and unmask channel
#define MASK                    BIT(2)

#define BIT(n) (1U<<(n))
#define IS_BIT_SET(value, pos) ((value)&BIT(pos))


typedef enum
{
    DMA_WRITE     = BIT(2), // Attention: Hardware -> RAM
    DMA_READ      = BIT(3), // Attention: RAM -> Hardware
    DMA_AUTOINIT  = BIT(4),
    DMA_INCREMENT = 0,
    DMA_DECREMENT = BIT(5),
    DMA_DEMAND    = 0,
    DMA_SINGLE    = BIT(6),
    DMA_BLOCK     = BIT(7)
} DMA_TRANSFERMODE_t;


typedef struct
{
    uint16_t pagePort;
    uint16_t addressPort;
    uint16_t counterPort;
    uint8_t  portNum;
} dma_channel_t;


extern const dma_channel_t dma_channel[4];


void dma_read (void* dest,      uint16_t length, const dma_channel_t* channel, DMA_TRANSFERMODE_t mode);
void dma_write(const void* src, uint16_t length, const dma_channel_t* channel, DMA_TRANSFERMODE_t mode);


#endif
