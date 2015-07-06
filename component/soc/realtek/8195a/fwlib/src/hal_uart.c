/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */


#include "rtl8195a.h"
#include "rtl8195a_uart.h"
#include "hal_uart.h"
#include "hal_gdma.h"

extern u32 _UartIrqHandle(VOID *Data);

#if (CONFIG_CHIP_A_CUT | CONFIG_CHIP_B_CUT)
HAL_Status 
HalRuartInitRtl8195a_Patch(
        IN VOID *Data  ///< RUART Adapter
        );
#else
extern _LONG_CALL_ HAL_Status
HalRuartInitRtl8195aV02(
        IN VOID *Data  ///< RUART Adapter
        );
#endif

const HAL_GDMA_CHNL Uart2_TX_GDMA_Chnl_Option[] = { 
    {0,0,GDMA0_CHANNEL0_IRQ,0},
    {0,1,GDMA0_CHANNEL1_IRQ,0},
    {0,2,GDMA0_CHANNEL2_IRQ,0},
    {0,3,GDMA0_CHANNEL3_IRQ,0},
    {0,4,GDMA0_CHANNEL4_IRQ,0},
    {0,5,GDMA0_CHANNEL5_IRQ,0},
    
    {0xff,0,0,0}    // end
};

const HAL_GDMA_CHNL Uart2_RX_GDMA_Chnl_Option[] = { 
    {1,0,GDMA1_CHANNEL0_IRQ,0},
    {1,1,GDMA1_CHANNEL1_IRQ,0},
    {1,2,GDMA1_CHANNEL2_IRQ,0},
    {1,3,GDMA1_CHANNEL3_IRQ,0},
    {1,4,GDMA1_CHANNEL4_IRQ,0},
    {1,5,GDMA1_CHANNEL5_IRQ,0},
    
    {0xff,0,0,0}    // end
};

VOID
HalRuartOpInit(
        IN VOID *Data
)
{
    PHAL_RUART_OP pHalRuartOp = (PHAL_RUART_OP) Data;

    pHalRuartOp->HalRuartAdapterLoadDef     = HalRuartAdapterLoadDefRtl8195a;
    pHalRuartOp->HalRuartTxGdmaLoadDef      = HalRuartTxGdmaLoadDefRtl8195a;
    pHalRuartOp->HalRuartRxGdmaLoadDef      = HalRuartRxGdmaLoadDefRtl8195a;
    pHalRuartOp->HalRuartResetRxFifo        = HalRuartResetRxFifoRtl8195a;
#if (CONFIG_CHIP_A_CUT | CONFIG_CHIP_B_CUT)
    pHalRuartOp->HalRuartInit               = HalRuartInitRtl8195a_Patch;         // Hardware Init ROM code patch 
#else
    pHalRuartOp->HalRuartInit               = HalRuartInitRtl8195aV02;         // Hardware Init
#endif
    pHalRuartOp->HalRuartDeInit             = HalRuartDeInitRtl8195a;         // Hardware Init
    pHalRuartOp->HalRuartPutC               = HalRuartPutCRtl8195a;         // Send a byte
    pHalRuartOp->HalRuartSend               = HalRuartSendRtl8195a;         // Polling mode Tx
    pHalRuartOp->HalRuartIntSend            = HalRuartIntSendRtl8195a;      // Interrupt mode Tx
    pHalRuartOp->HalRuartDmaSend            = HalRuartDmaSendRtl8195a;      // DMA mode Tx
    pHalRuartOp->HalRuartStopSend           = HalRuartStopSendRtl8195a;     // Stop non-blocking TX
    pHalRuartOp->HalRuartGetC               = HalRuartGetCRtl8195a;           // get a byte
    pHalRuartOp->HalRuartRecv               = HalRuartRecvRtl8195a;         // Polling mode Rx
    pHalRuartOp->HalRuartIntRecv            = HalRuartIntRecvRtl8195a;      // Interrupt mode Rx
    pHalRuartOp->HalRuartDmaRecv            = HalRuartDmaRecvRtl8195a;      // DMA mode Rx
    pHalRuartOp->HalRuartStopRecv           = HalRuartStopRecvRtl8195a;     // Stop non-blocking Rx
    pHalRuartOp->HalRuartGetIMR             = HalRuartGetIMRRtl8195a;
    pHalRuartOp->HalRuartSetIMR             = HalRuartSetIMRRtl8195a;
    pHalRuartOp->HalRuartGetDebugValue      = HalRuartGetDebugValueRtl8195a;
    pHalRuartOp->HalRuartDmaInit            = HalRuartDmaInitRtl8195a;
    pHalRuartOp->HalRuartRTSCtrl            = HalRuartRTSCtrlRtl8195a;
    pHalRuartOp->HalRuartRegIrq             = HalRuartRegIrqRtl8195a;
    pHalRuartOp->HalRuartIntEnable          = HalRuartIntEnableRtl8195a;
    pHalRuartOp->HalRuartIntDisable         = HalRuartIntDisableRtl8195a;
}



/**
 * Load UART HAL default setting
 *
 * Call this function to load the default setting for UART HAL adapter
 *
 *
 */
VOID
HalRuartAdapterInit(
    PRUART_ADAPTER pRuartAdapter,
    u8 UartIdx
)
{
    PHAL_RUART_OP pHalRuartOp;
    PHAL_RUART_ADAPTER pHalRuartAdapter;
    
    if (NULL == pRuartAdapter) {
        return;
    }
    
    pHalRuartOp = pRuartAdapter->pHalRuartOp;
    pHalRuartAdapter = pRuartAdapter->pHalRuartAdapter;

    if ((NULL == pHalRuartOp) || (NULL == pHalRuartAdapter)) {
        return;
    }

    // Load default setting
    if (pHalRuartOp->HalRuartAdapterLoadDef != NULL) {
        pHalRuartOp->HalRuartAdapterLoadDef (pHalRuartAdapter, UartIdx);
    }
    else {
        // Initial your UART HAL adapter here
    }

    // Start to modify the defualt setting
    pHalRuartAdapter->PinmuxSelect = RUART0_MUX_TO_GPIOC;
    pHalRuartAdapter->BaudRate = 38400;

//    pHalRuartAdapter->IrqHandle.IrqFun = (IRQ_FUN)_UartIrqHandle;
//    pHalRuartAdapter->IrqHandle.Data = (void *)pHalRuartAdapter;
//    pHalRuartAdapter->IrqHandle.Priority = 0x20;

    // Register IRQ
    InterruptRegister(&pHalRuartAdapter->IrqHandle);
    
}

/**
 * Load UART HAL GDMA default setting
 *
 * Call this function to load the default setting for UART GDMA
 *
 *
 */
HAL_Status
HalRuartTxGdmaInit(
    PHAL_RUART_OP pHalRuartOp,
    PHAL_RUART_ADAPTER pHalRuartAdapter,
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    u8 gdma_idx;
    u8 gdma_chnum;
    HAL_GDMA_CHNL *pgdma_chnl;
    
    if ((NULL == pHalRuartOp) || (NULL == pHalRuartAdapter) || (NULL == pUartGdmaConfig)) {
        return HAL_ERR_PARA;
    }

    // Load default setting
    if (pHalRuartOp->HalRuartTxGdmaLoadDef != NULL) {
        pHalRuartOp->HalRuartTxGdmaLoadDef (pHalRuartAdapter, pUartGdmaConfig);
    }
    else {
        // Initial your GDMA setting here
    }

    // Start to patch the default setting
    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pTxHalGdmaAdapter;
    if (HalGdmaChnlRegister(pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum) != HAL_OK) {
        // The default GDMA Channel is not available, try others
        if (pHalRuartAdapter->UartIndex == 2) {
            // UART2 TX Only can use GDMA 0
            pgdma_chnl = HalGdmaChnlAlloc((HAL_GDMA_CHNL*)Uart2_TX_GDMA_Chnl_Option);
        }
        else {
            pgdma_chnl = HalGdmaChnlAlloc(NULL);
        }

        if (pgdma_chnl == NULL) {
            // No Available DMA channel
            return HAL_BUSY;
        }
        else {
            pHalGdmaAdapter->GdmaIndex   = pgdma_chnl->GdmaIndx;
            pHalGdmaAdapter->ChNum       = pgdma_chnl->GdmaChnl;
            pHalGdmaAdapter->ChEn        = 0x0101 << pgdma_chnl->GdmaChnl;
            pUartGdmaConfig->TxGdmaIrqHandle.IrqNum = pgdma_chnl->IrqNum;            
        }
    }

    // User can assign a Interrupt Handler here
//    pUartGdmaConfig->TxGdmaIrqHandle.Data = pHalRuartAdapter;
//    pUartGdmaConfig->TxGdmaIrqHandle.IrqFun = (IRQ_FUN)_UartTxDmaIrqHandle
//    pUartGdmaConfig->TxGdmaIrqHandle.Priority = 0x20;

    pHalRuartOp->HalRuartDmaInit (pHalRuartAdapter);
    InterruptRegister(&pUartGdmaConfig->TxGdmaIrqHandle);
    InterruptEn(&pUartGdmaConfig->TxGdmaIrqHandle);

    return HAL_OK;
}

VOID
HalRuartTxGdmaDeInit(
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL GdmaChnl;

    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pTxHalGdmaAdapter;
    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pUartGdmaConfig->TxGdmaIrqHandle.IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}

/**
 * Load UART HAL GDMA default setting
 *
 * Call this function to load the default setting for UART GDMA
 *
 *
 */
HAL_Status
HalRuartRxGdmaInit(
    PHAL_RUART_OP pHalRuartOp,
    PHAL_RUART_ADAPTER pHalRuartAdapter,
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL *pgdma_chnl;
    
    if ((NULL == pHalRuartOp) || (NULL == pHalRuartAdapter) || (NULL == pUartGdmaConfig)) {
        return HAL_ERR_PARA;
    }

    // Load default setting
    if (pHalRuartOp->HalRuartRxGdmaLoadDef != NULL) {
        pHalRuartOp->HalRuartRxGdmaLoadDef (pHalRuartAdapter, pUartGdmaConfig);
    }
    else {
        // Initial your GDMA setting here
    }

    // Start to patch the default setting
    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pRxHalGdmaAdapter;
    if (HalGdmaChnlRegister(pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum) != HAL_OK) {
        // The default GDMA Channel is not available, try others
        if (pHalRuartAdapter->UartIndex == 2) {
            // UART2 RX Only can use GDMA 1
            pgdma_chnl = HalGdmaChnlAlloc((HAL_GDMA_CHNL*)Uart2_RX_GDMA_Chnl_Option);
        }
        else {
            pgdma_chnl = HalGdmaChnlAlloc(NULL);
        }

        if (pgdma_chnl == NULL) {
            // No Available DMA channel
            return HAL_BUSY;
        }
        else {
            pHalGdmaAdapter->GdmaIndex   = pgdma_chnl->GdmaIndx;
            pHalGdmaAdapter->ChNum       = pgdma_chnl->GdmaChnl;
            pHalGdmaAdapter->ChEn        = 0x0101 << pgdma_chnl->GdmaChnl;
            pUartGdmaConfig->RxGdmaIrqHandle.IrqNum = pgdma_chnl->IrqNum;            
        }
    }

//    pUartGdmaConfig->RxGdmaIrqHandle.Data = pHalRuartAdapter;
//    pUartGdmaConfig->RxGdmaIrqHandle.IrqFun = (IRQ_FUN)_UartTxDmaIrqHandle;
//    pUartGdmaConfig->RxGdmaIrqHandle.Priority = 0x20;

    pHalRuartOp->HalRuartDmaInit (pHalRuartAdapter);
    InterruptRegister(&pUartGdmaConfig->RxGdmaIrqHandle);
    InterruptEn(&pUartGdmaConfig->RxGdmaIrqHandle);

    return HAL_OK;
}

VOID
HalRuartRxGdmaDeInit(
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL GdmaChnl;

    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pRxHalGdmaAdapter;
    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pUartGdmaConfig->RxGdmaIrqHandle.IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}

/**
 * Hook a RX indication callback
 *
 * To hook a callback function which will be called when a got a RX byte
 *
 *
 */
VOID
HalRuartRxIndHook(
    PRUART_ADAPTER pRuartAdapter,
    VOID *pCallback,
    VOID *pPara
)
{
    PHAL_RUART_ADAPTER pHalRuartAdapter = pRuartAdapter->pHalRuartAdapter;

    pHalRuartAdapter->RxDRCallback = (void (*)(void*))pCallback;
    pHalRuartAdapter->RxDRCbPara = pPara;

    // enable RX data ready interrupt
    pHalRuartAdapter->Interrupts |= RUART_IER_ERBI | RUART_IER_ELSI;
    pRuartAdapter->pHalRuartOp->HalRuartSetIMR(pHalRuartAdapter);    
}


HAL_Status
HalRuartResetTxFifo(
    IN VOID *Data
)
{
    return (HalRuartResetTxFifoRtl8195a(Data));
}

