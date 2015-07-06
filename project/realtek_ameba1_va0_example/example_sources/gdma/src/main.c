/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2015 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "device.h"
#include "diag.h"
#include "main.h"

#include "dma_api.h"

#define DMA_CPY_LEN     256
#define DMA_SRC_OFFSET  0
#define DMA_DST_OFFSET  0

gdma_t gdma;
uint8_t TestBuf1[512];
uint8_t TestBuf2[512];
volatile uint8_t dma_done;


void dma_done_handler(uint32_t id) {
    DiagPrintf("DMA Copy Done!!\r\n");
    dma_done = 1;
}

void main(void) {
    int i;
    int err;

    dma_memcpy_init(&gdma, dma_done_handler, (uint32_t)&gdma);
    for (i=0;i< 512;i++) {
        TestBuf1[i] = i;
    }
    _memset(TestBuf2, 0xff, 512);

    dma_done = 0;
    dma_memcpy(&gdma, TestBuf2+DMA_DST_OFFSET, TestBuf1+DMA_SRC_OFFSET, DMA_CPY_LEN);

    while (dma_done == 0);

    err = 0;
    for (i=0;i<DMA_CPY_LEN;i++) {
        if (TestBuf2[i+DMA_DST_OFFSET] != TestBuf1[i+DMA_SRC_OFFSET]) {
            DiagPrintf("DMA Copy Memory Compare Err, %d %x %x\r\n", i, TestBuf1[i+DMA_SRC_OFFSET], TestBuf2[i+DMA_DST_OFFSET]);
            err = 1;
            break;
        }
    }
    
    if (!err) {
        DiagPrintf("DMA Copy Memory Compare OK!! %x\r\n", TestBuf2[DMA_DST_OFFSET+DMA_CPY_LEN]);
    }
    
    while(1);
}
