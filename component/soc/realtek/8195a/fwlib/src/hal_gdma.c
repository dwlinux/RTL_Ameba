/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */


#include "rtl8195a.h"
#include "hal_gdma.h"

#define MAX_GDMA_INDX       1
#define MAX_GDMA_CHNL       6

static u8 HalGdmaReg[MAX_GDMA_INDX+1];

const HAL_GDMA_CHNL GDMA_Chnl_Option[] = { 
        {0,0,GDMA0_CHANNEL0_IRQ,0},
        {1,0,GDMA1_CHANNEL0_IRQ,0},
        {0,1,GDMA0_CHANNEL1_IRQ,0},
        {1,1,GDMA1_CHANNEL1_IRQ,0},
        {0,2,GDMA0_CHANNEL2_IRQ,0},
        {1,2,GDMA1_CHANNEL2_IRQ,0},
        {0,3,GDMA0_CHANNEL3_IRQ,0},
        {1,3,GDMA1_CHANNEL3_IRQ,0},
        {0,4,GDMA0_CHANNEL4_IRQ,0},
        {1,4,GDMA1_CHANNEL4_IRQ,0},
        {0,5,GDMA0_CHANNEL5_IRQ,0},
        {1,5,GDMA1_CHANNEL5_IRQ,0},
        
        {0xff,0,0,0}    // end
};

const u16 HalGdmaChnlEn[6] = {
    GdmaCh0, GdmaCh1, GdmaCh2, GdmaCh3,
    GdmaCh4, GdmaCh5
};
    
VOID HalGdmaOpInit(
    IN  VOID *Data
)
{
    PHAL_GDMA_OP pHalGdmaOp = (PHAL_GDMA_OP) Data;

    pHalGdmaOp->HalGdmaOnOff = HalGdmaOnOffRtl8195a;
    pHalGdmaOp->HalGdamChInit = HalGdamChInitRtl8195a;
    pHalGdmaOp->HalGdmaChDis = HalGdmaChDisRtl8195a;
    pHalGdmaOp->HalGdmaChEn = HalGdmaChEnRtl8195a;
    pHalGdmaOp->HalGdmaChSeting = HalGdmaChSetingRtl8195a;
    pHalGdmaOp->HalGdmaChBlockSeting = HalGdmaChBlockSetingRtl8195a;
    pHalGdmaOp->HalGdmaChIsrEnAndDis = HalGdmaChIsrEnAndDisRtl8195a;
    pHalGdmaOp->HalGdmaChIsrClean = HalGdmaChIsrCleanRtl8195a;
    pHalGdmaOp->HalGdmaChCleanAutoSrc = HalGdmaChCleanAutoSrcRtl8195a;
    pHalGdmaOp->HalGdmaChCleanAutoDst = HalGdmaChCleanAutoDstRtl8195a;
}

VOID HalGdmaOn(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    pHalGdmaAdapter->GdmaOnOff = ON;
    HalGdmaOnOffRtl8195a((VOID*)pHalGdmaAdapter);
}

VOID HalGdmaOff(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    pHalGdmaAdapter->GdmaOnOff = OFF;
    HalGdmaOnOffRtl8195a((VOID*)pHalGdmaAdapter);
}

BOOL HalGdmaChInit(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    return (HalGdamChInitRtl8195a((VOID*)pHalGdmaAdapter));
}

VOID HalGdmaChDis(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    HalGdmaChDisRtl8195a((VOID*)pHalGdmaAdapter);
}

VOID HalGdmaChEn(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    HalGdmaChEnRtl8195a((VOID*)pHalGdmaAdapter);
}

BOOL HalGdmaChSeting(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    return (HalGdmaChSetingRtl8195a((VOID*)pHalGdmaAdapter));
}

BOOL HalGdmaChBlockSeting(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    return (HalGdmaChBlockSetingRtl8195a((VOID*)pHalGdmaAdapter));
}

VOID HalGdmaChIsrEn(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    pHalGdmaAdapter->IsrCtrl = ENABLE;
    HalGdmaChIsrEnAndDisRtl8195a((VOID*)pHalGdmaAdapter);
}

VOID HalGdmaChIsrDis(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    pHalGdmaAdapter->IsrCtrl = DISABLE;
    HalGdmaChIsrEnAndDisRtl8195a((VOID*)pHalGdmaAdapter);
}

u8 HalGdmaChIsrClean(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    return (HalGdmaChIsrCleanRtl8195a((VOID*)pHalGdmaAdapter));
}

VOID HalGdmaChCleanAutoSrc(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    HalGdmaChCleanAutoSrcRtl8195a((VOID*)pHalGdmaAdapter);
}

VOID HalGdmaChCleanAutoDst(PHAL_GDMA_ADAPTER pHalGdmaAdapter)
{
    HalGdmaChCleanAutoDstRtl8195a((VOID*)pHalGdmaAdapter);
}

HAL_Status HalGdmaChnlRegister (u8 GdmaIdx, u8 ChnlNum)
{
    u32 mask;

    if ((GdmaIdx > MAX_GDMA_INDX) || (ChnlNum > MAX_GDMA_CHNL)) {
        // Invalid GDMA Index or Channel Number
        return HAL_ERR_PARA;
    }

    mask = 1 << ChnlNum;

    if ((HalGdmaReg[GdmaIdx] & mask) != 0) {
        return HAL_BUSY;
    }
    else {
#if 1
        if (HalGdmaReg[GdmaIdx] == 0) {
            if (GdmaIdx == 0) {
                ACTCK_GDMA0_CCTRL(ON);
                GDMA0_FCTRL(ON);
            }
            else {
                ACTCK_GDMA1_CCTRL(ON);
                GDMA1_FCTRL(ON);
            }
        }
#endif    
        HalGdmaReg[GdmaIdx] |= mask;
        return HAL_OK;
    }
}

VOID HalGdmaChnlUnRegister (u8 GdmaIdx, u8 ChnlNum)
{
    u32 mask;

    if ((GdmaIdx > MAX_GDMA_INDX) || (ChnlNum > MAX_GDMA_CHNL)) {
        // Invalid GDMA Index or Channel Number
        return;
    }
    
    mask = 1 << ChnlNum;

    HalGdmaReg[GdmaIdx] &= ~mask;
#if 1
    if (HalGdmaReg[GdmaIdx] == 0) {
        if (GdmaIdx == 0) {
            ACTCK_GDMA0_CCTRL(OFF);
            GDMA0_FCTRL(OFF);
        }
        else {
            ACTCK_GDMA1_CCTRL(OFF);
            GDMA1_FCTRL(OFF);
        }
    }
#endif    
}

PHAL_GDMA_CHNL HalGdmaChnlAlloc (HAL_GDMA_CHNL *pChnlOption)
{
    HAL_GDMA_CHNL *pgdma_chnl;

    pgdma_chnl = pChnlOption;
    if (pChnlOption == NULL) {
        // Use default GDMA Channel Option table
        pgdma_chnl = (HAL_GDMA_CHNL*)&GDMA_Chnl_Option[0];
    }

    while (pgdma_chnl->GdmaIndx <= MAX_GDMA_INDX) {
        if (HalGdmaChnlRegister(pgdma_chnl->GdmaIndx, pgdma_chnl->GdmaChnl) == HAL_OK) {
            // This GDMA Channel is available
            break;
        }
        pgdma_chnl += 1;
    }

    if (pgdma_chnl->GdmaIndx > MAX_GDMA_INDX) {
        pgdma_chnl = NULL;
    }
    
    return pgdma_chnl;
}

VOID HalGdmaChnlFree (HAL_GDMA_CHNL *pChnl)
{
    IRQ_HANDLE IrqHandle;

    IrqHandle.IrqNum = pChnl->IrqNum;
    InterruptDis(&IrqHandle);
    InterruptUnRegister(&IrqHandle);
    HalGdmaChnlUnRegister(pChnl->GdmaIndx, pChnl->GdmaChnl);
}

VOID HalGdmaMemIrqHandler(VOID *pData)
{
    PHAL_GDMA_OBJ pHalGdmaObj=(PHAL_GDMA_OBJ)pData;
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    PIRQ_HANDLE pGdmaIrqHandle;
    u8 IsrTypeMap;
    
    pHalGdmaAdapter = &(pHalGdmaObj->HalGdmaAdapter);
    pGdmaIrqHandle = &(pHalGdmaObj->GdmaIrqHandle);
    // Clean Auto Reload Bit
    HalGdmaChCleanAutoDst((VOID*)pHalGdmaAdapter);

    // Clear Pending ISR
    IsrTypeMap = HalGdmaChIsrClean((VOID*)pHalGdmaAdapter);

    HalGdmaChDis((VOID*)(pHalGdmaAdapter));
    pHalGdmaObj->Busy = 0;

    if (pGdmaIrqHandle->IrqFun != NULL) {
        pGdmaIrqHandle->IrqFun((VOID*)pGdmaIrqHandle->Data);
    }
}

BOOL HalGdmaMemCpyInit(PHAL_GDMA_OBJ pHalGdmaObj)
{
    HAL_GDMA_CHNL *pgdma_chnl;
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    PIRQ_HANDLE pGdmaIrqHandle;
    IRQ_HANDLE IrqHandle;

    pgdma_chnl = HalGdmaChnlAlloc(NULL);    // get a whatever GDMA channel
    if (NULL == pgdma_chnl) {        
        DBG_GDMA_ERR("%s: Cannot allocate a GDMA Channel\n", __FUNCTION__);
        return _FALSE;
    }

    pHalGdmaAdapter = &(pHalGdmaObj->HalGdmaAdapter);
    pGdmaIrqHandle = &(pHalGdmaObj->GdmaIrqHandle);
    
    DBG_GDMA_INFO("%s: Use GDMA%d CH%d\n", __FUNCTION__, pgdma_chnl->GdmaIndx, pgdma_chnl->GdmaChnl);
#if 0
    if (pgdma_chnl->GdmaIndx == 0) {
        ACTCK_GDMA0_CCTRL(ON);
        GDMA0_FCTRL(ON);
    }
    else if (pgdma_chnl->GdmaIndx == 1) {
        ACTCK_GDMA1_CCTRL(ON);
        GDMA1_FCTRL(ON);
    }
#endif    
    _memset((void *)pHalGdmaAdapter, 0, sizeof(HAL_GDMA_ADAPTER));

//    pHalGdmaAdapter->GdmaCtl.TtFc = TTFCMemToMem;
    pHalGdmaAdapter->GdmaCtl.Done = 1;
//    pHalGdmaAdapter->MuliBlockCunt = 0;
//    pHalGdmaAdapter->MaxMuliBlock = 1;
    pHalGdmaAdapter->ChNum = pgdma_chnl->GdmaChnl;
    pHalGdmaAdapter->GdmaIndex = pgdma_chnl->GdmaIndx;
    pHalGdmaAdapter->ChEn = 0x0101 << pgdma_chnl->GdmaChnl;
    pHalGdmaAdapter->GdmaIsrType = (TransferType|ErrType);
    pHalGdmaAdapter->IsrCtrl = ENABLE;
    pHalGdmaAdapter->GdmaOnOff = ON;

    pHalGdmaAdapter->GdmaCtl.IntEn      = 1;
//    pHalGdmaAdapter->GdmaCtl.SrcMsize   = MsizeEight;
//    pHalGdmaAdapter->GdmaCtl.DestMsize  = MsizeEight;
//    pHalGdmaAdapter->GdmaCtl.SrcTrWidth = TrWidthFourBytes;
//    pHalGdmaAdapter->GdmaCtl.DstTrWidth = TrWidthFourBytes;
//    pHalGdmaAdapter->GdmaCtl.Dinc = IncType;
//    pHalGdmaAdapter->GdmaCtl.Sinc = IncType;

    pGdmaIrqHandle->IrqNum = pgdma_chnl->IrqNum;
    pGdmaIrqHandle->Priority = 10;

    IrqHandle.IrqFun = (IRQ_FUN) HalGdmaMemIrqHandler;
    IrqHandle.Data = (u32) pHalGdmaObj;
    IrqHandle.IrqNum = pGdmaIrqHandle->IrqNum;
    IrqHandle.Priority = pGdmaIrqHandle->Priority;

    InterruptRegister(&IrqHandle);
    InterruptEn(&IrqHandle);
    pHalGdmaObj->Busy = 0;
    
    return _TRUE;
}

VOID HalGdmaMemCpyDeInit(PHAL_GDMA_OBJ pHalGdmaObj)
{
    HAL_GDMA_CHNL GdmaChnl;
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    PIRQ_HANDLE pGdmaIrqHandle;

    pHalGdmaAdapter = &(pHalGdmaObj->HalGdmaAdapter);
    pGdmaIrqHandle = &(pHalGdmaObj->GdmaIrqHandle);

    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pGdmaIrqHandle->IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}

// If multi-task using the same GDMA Object, then it needs a mutex to protect this procedure
VOID* HalGdmaMemCpy(PHAL_GDMA_OBJ pHalGdmaObj, void* pDest, void* pSrc, u32 len)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;

    if (pHalGdmaObj->Busy) {
        DBG_GDMA_ERR("%s: ==> GDMA is Busy\r\n", __FUNCTION__);
        return 0;
    }
    pHalGdmaObj->Busy = 1;
    pHalGdmaAdapter = &(pHalGdmaObj->HalGdmaAdapter);

    DBG_GDMA_INFO("%s: ==> Src=0x%x Dst=0x%x Len=%d\r\n", __FUNCTION__, pSrc, pDest, len);
    if ((((u32)pSrc & 0x03)==0) &&
        (((u32)pDest & 0x03)==0) &&        
        ((len & 0x03)== 0)) {
        // 4-bytes aligned, move 4 bytes each transfer
        pHalGdmaAdapter->GdmaCtl.SrcMsize   = MsizeEight;
        pHalGdmaAdapter->GdmaCtl.SrcTrWidth = TrWidthFourBytes;
        pHalGdmaAdapter->GdmaCtl.DestMsize = MsizeEight;
        pHalGdmaAdapter->GdmaCtl.DstTrWidth = TrWidthFourBytes;
        pHalGdmaAdapter->GdmaCtl.BlockSize = len >> 2;
    }
    else {
        pHalGdmaAdapter->GdmaCtl.SrcMsize   = MsizeEight;
        pHalGdmaAdapter->GdmaCtl.SrcTrWidth = TrWidthOneByte;
        pHalGdmaAdapter->GdmaCtl.DestMsize = MsizeEight;
        pHalGdmaAdapter->GdmaCtl.DstTrWidth = TrWidthOneByte;
        pHalGdmaAdapter->GdmaCtl.BlockSize = len;
    }

    pHalGdmaAdapter->ChSar = (u32)pSrc;
    pHalGdmaAdapter->ChDar = (u32)pDest;                
    pHalGdmaAdapter->PacketLen = len;

    HalGdmaOn((pHalGdmaAdapter));
    HalGdmaChIsrEn((pHalGdmaAdapter));
    HalGdmaChSeting((pHalGdmaAdapter));
    HalGdmaChEn((pHalGdmaAdapter));

    return (pDest);
}
