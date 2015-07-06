/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */


#include "rtl8195a.h"
#include "rtl8195a_mii.h"
#include "hal_mii.h"



VOID MiiIrqHandle (IN VOID *Data);

VOID MiiIrqHandle (IN VOID *Data) {
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    u32 RegValue = HalMiiGmacGetInterruptStatusRtl8195a();
#ifdef CONFIG_MII_VERIFY
	extern u8 isRxOK;
	extern u8 isTxOK;
	extern u8 RxIntCnt;
	

//	DBG_8195A("ISR = 0x%08X\n", RegValue);
	if(RegValue & GMAC_ISR_ROK) {
		HalMiiGmacClearInterruptStatusRtl8195a(0x00410001);
		isRxOK = 1;
		RxIntCnt++;
	}

	if(RegValue & GMAC_ISR_TOK_TI) {
		HalMiiGmacClearInterruptStatusRtl8195a(0x00410040);
		isTxOK = 1;
    }
#else

#endif
}


VOID
ConfigDebugPort_E4(u32 DebugSelect) {
    u32 RegValue;
    BOOL DebugMsg = _FALSE;

    if (DebugMsg) {
        LOGI(ANSI_COLOR_YELLOW"Debug Port Select (0xE4)\n"ANSI_COLOR_RESET);
        LOGD2("[P] PERI_ON_010: %X\n", HAL_READ32(PERI_ON_BASE, REG_SOC_FUNC_EN));
        LOGD2("[P] PERI_ON_014: %X\n", HAL_READ32(PERI_ON_BASE, REG_SOC_HCI_COM_FUNC_EN));
    }

    RegValue = HAL_MII_READ32(REG_RTL_MII_CCR);
    if (DebugMsg) {
        LOGD2("[B] 0x00E4: %X\n", RegValue);
    }

    RegValue |= DebugSelect << 2;
    if (DebugMsg) {
        LOGD2("[B] RegValue: %X\n", RegValue);
    }

    HAL_MII_WRITE32(REG_RTL_MII_CCR, RegValue);
    if (DebugMsg) {
        LOGD2("[A] 0x00E4: %X\n", HAL_MII_READ32(REG_RTL_MII_CCR));
    }
}


/**
 * MII Initialize.
 *
 * MII Initialize.
 *
 * Initialization Steps:
 *   I. Rtl8195A Board Configurations:
 *     1. MII Function Enable & AHB mux
 *
 * @return runtime status value.
 */
BOOL
HalMiiGmacInitRtl8195a(
        IN VOID *Data
        )
{
	PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
	PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
	u32 RegValue;


	/* 1. enable MII Pinmux & disable SDIO Host/Device mode Pinmux */
	RegValue = HAL_READ32(PERI_ON_BASE, REG_HCI_PINMUX_CTRL);
	RegValue |= BIT24;
	RegValue &= ~(BIT0 | BIT1); // Important!
	HAL_WRITE32(PERI_ON_BASE, REG_HCI_PINMUX_CTRL, RegValue);

	/* 2. enable MII IP block (214, 12) */
	RegValue = HAL_READ32(PERI_ON_BASE, REG_SOC_HCI_COM_FUNC_EN);
	RegValue |= BIT12;
	HAL_WRITE32(PERI_ON_BASE, REG_SOC_HCI_COM_FUNC_EN, RegValue);

	/* 3. Lexra2AHB Function Enable (304, 11) */
	RegValue = HAL_READ32(PERI_ON_BASE, REG_PESOC_SOC_CTRL);
	RegValue |= BIT11;
	HAL_WRITE32(PERI_ON_BASE, REG_PESOC_SOC_CTRL, RegValue);

	/* 4. enable MII bus clock (240, 24|25) */
	RegValue = HAL_READ32(PERI_ON_BASE, REG_PESOC_HCI_CLK_CTRL0);
	RegValue |= (BIT24 | BIT25);
	HAL_WRITE32(PERI_ON_BASE, REG_PESOC_HCI_CLK_CTRL0, RegValue);

	/* 5. */
	RegValue = HAL_READ32(SYSTEM_CTRL_BASE, 0x74) & 0xFFFFC7FF;
	HAL_WRITE32(SYSTEM_CTRL_BASE, 0x74, (RegValue | 0x00003000));

	/* 6. AHB mux: select MII (214, 13) */
    	RegValue = HAL_READ32(PERI_ON_BASE, REG_SOC_HCI_COM_FUNC_EN);
	RegValue |= BIT13;
	HAL_WRITE32(PERI_ON_BASE, REG_SOC_HCI_COM_FUNC_EN, RegValue);

	/* 7. Vendor Register Clock Enable (230, 6|7) */
	RegValue = HAL_READ32(PERI_ON_BASE, REG_PESOC_CLK_CTRL);
	RegValue |= (BIT6 | BIT7);
	HAL_WRITE32(PERI_ON_BASE, REG_PESOC_CLK_CTRL, RegValue);

	/* 8. Enable GMAC Lexra Timeout (090, 16|17|18) */
	RegValue = HAL_READ32(VENDOR_REG_BASE, 0x0090);
	RegValue |= (BIT16 | BIT17 | BIT18);
	HAL_WRITE32(VENDOR_REG_BASE, 0x0090, RegValue);

	/* 9. Endian Swap Control (304, 12|13) */
	RegValue = HAL_READ32(PERI_ON_BASE, REG_PESOC_SOC_CTRL);
	RegValue |= (BIT12 | BIT13);
	HAL_WRITE32(PERI_ON_BASE, REG_PESOC_SOC_CTRL, RegValue);

	return _TRUE;
}


BOOL
HalMiiInitRtl8195a(
        IN VOID *Data
        )
{
    DBG_ENTRANCE;
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    u32 RegValue;

    return _TRUE;
}


BOOL
HalMiiGmacResetRtl8195a(
        IN VOID *Data
        )
{
	HAL_MII_WRITE32(REG_RTL_MII_CR, (HAL_MII_READ32(REG_RTL_MII_CR) | BIT0));

    return _TRUE;
}


BOOL
HalMiiGmacEnablePhyModeRtl8195a(
        IN VOID *Data
        )
{
    return _TRUE;
}


u32
HalMiiGmacXmitRtl8195a(
        IN VOID *Data
        )
{
    return 0;
}


VOID
HalMiiGmacCleanTxRingRtl8195a(
        IN VOID *Data
        )
{
}


VOID
HalMiiGmacFillTxInfoRtl8195a(
        IN VOID *Data
        )
{
    DBG_ENTRANCE;
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    PTX_INFO pTx_Info = pMiiAdapter->pTx_Info;
    VOID* TxBuffer  = pMiiAdapter->TxBuffer;
    u32 RegValue;

    LOGI(ANSI_COLOR_GREEN"==[ Tx Descriptor Configuration ]======\n"ANSI_COLOR_RESET);

    pTx_Info->opts1.dw = 0xBC8001FE;
    /* pTx_Info->opts1.dw = 0xBC800080; // size: 128 */

    pTx_Info->addr     = (u32)TxBuffer;
    pTx_Info->opts2.dw = 0x0400279F;
    pTx_Info->opts3.dw = 0x00000000;
    /* pTx_Info->opts4.dw = 0x57800000; */
    pTx_Info->opts4.dw = 0x1FE00000;
    LOGI("pTx_Info->addr: %X\n", pTx_Info->addr);

    LOGI(ANSI_COLOR_YELLOW"TxFDP1 Register(0x%04X) (W: 0x%X)\n"ANSI_COLOR_RESET, REG_RTL_MII_TXFDP1, pTx_Info);
    RegValue = HAL_MII_READ32(REG_RTL_MII_TXFDP1);
    LOGD2("[B] REG_RTL_MII_TXFDP1: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_TXFDP1, pTx_Info);
    LOGD2("[A] REG_RTL_MII_TXFDP1: %X\n", HAL_MII_READ32(REG_RTL_MII_TXFDP1));
}


VOID
HalMiiGmacFillRxInfoRtl8195a(
        IN VOID *Data
        )
{
    DBG_ENTRANCE;
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    PRX_INFO pRx_Info = pMiiAdapter->pRx_Info;
    VOID* RxBuffer    = pMiiAdapter->RxBuffer;
    u32 RegValue;

    LOGI(ANSI_COLOR_GREEN"==[ Rx Descriptor Configuration ]======\n"ANSI_COLOR_RESET);

    /* pRx_Info->opts1.dw = 0x80000200; //Data Length: 4095(FFF), 512(200) */
    pRx_Info->opts1.dw = 0x800001FC; //Data Length: 4095(FFF), 512(200)
    /* pRx_Info->opts1.dw = 0x8000007F; */

    pRx_Info->addr     = (u32)RxBuffer;
    pRx_Info->opts2.dw = 0x00000000;
    pRx_Info->opts3.dw = 0x00000000;

    LOGI(ANSI_COLOR_YELLOW"RxFDP1 Register(0x%04X) (W: 0x%X)\n"ANSI_COLOR_RESET, REG_RTL_MII_RXFDP1, pRx_Info);
    RegValue = HAL_MII_READ32(REG_RTL_MII_RXFDP1);
    LOGD2("[B] REG_RTL_MII_RXFDP1: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_RXFDP1, pRx_Info);
    LOGD2("[A] REG_RTL_MII_RXFDP1: %X\n", HAL_MII_READ32(REG_RTL_MII_RXFDP1));
}


VOID
HalMiiGmacTxRtl8195a(
        IN VOID *Data
        )
{
    DBG_ENTRANCE;
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    PTX_INFO pTx_Info = pMiiAdapter->pTx_Info;
    u32 RegValue;

    LOGI(ANSI_COLOR_GREEN"==[ Tx ]===============================\n"ANSI_COLOR_RESET);

    LOGI(ANSI_COLOR_YELLOW"REG_RTL_MII_IOCMD Register(0x%04X): Enable Tx (W: 0x10, BIT4)\n"ANSI_COLOR_RESET, REG_RTL_MII_IOCMD);
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD);
    LOGD2("[B] REG_RTL_MII_IOCMD: %X\n", RegValue);
    RegValue |= BIT_IOCMD_TXENABLE(1);
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD, RegValue);
    LOGD2("[A] REG_RTL_MII_IOCMD: %X\n", HAL_MII_READ32(REG_RTL_MII_IOCMD));
    LOGD4("--> Tx Descriptor opts1: %X\n", HAL_READ32(pTx_Info, 0));

    LOGI(ANSI_COLOR_YELLOW"REG_RTL_MII_IOCMD Register(0x%04X): Enable 1st Tx (W: 0x1, BIT0)\n"ANSI_COLOR_RESET, REG_RTL_MII_IOCMD);
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD);
    LOGD2("[B] REG_RTL_MII_IOCMD: %X\n", RegValue);
    RegValue |= BIT_IOCMD_FIRST_DMATX_ENABLE(1);
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD, RegValue);
    LOGD2("[A] REG_RTL_MII_IOCMD: %X\n", HAL_MII_READ32(REG_RTL_MII_IOCMD));
    LOGD4("--> Tx Descriptor opts1: %X\n", HAL_READ32(pTx_Info, 0));
}


VOID
HalMiiGmacRxRtl8195a(
        IN VOID *Data
        )
{
    DBG_ENTRANCE;
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    u32 RegValue;

    LOGI(ANSI_COLOR_GREEN"==[ Rx ]===============================\n"ANSI_COLOR_RESET);
    LOGI(ANSI_COLOR_YELLOW"Transmit (Tx) Configuration Register (0x%04X) (W: 0x00000C00)\n"ANSI_COLOR_RESET, REG_RTL_MII_TCR);
    RegValue = HAL_MII_READ32(REG_RTL_MII_TCR);
    LOGD2("[B] REG_RTL_MII_TCR: %X\n", RegValue);

    HAL_MII_WRITE32(REG_RTL_MII_TCR, 0x00000D00);  // loopback R2T mode
    LOGD2("[A] REG_RTL_MII_TCR: %X\n", HAL_MII_READ32(REG_RTL_MII_TCR));

    LOGI(ANSI_COLOR_YELLOW"Receive (Rx) Configuration Register (0x%04X) (W: 0x0000007F)\n"ANSI_COLOR_RESET, REG_RTL_MII_RCR);
    RegValue = HAL_MII_READ32(REG_RTL_MII_RCR);
    LOGD2("[B] REG_RTL_MII_RCR: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_RCR, 0x0000007F);
    LOGD2("[A] REG_RTL_MII_RCR: %X\n", HAL_MII_READ32(REG_RTL_MII_RCR));

    LOGI(ANSI_COLOR_YELLOW"EhtrntRxCPU_Des_Num1 (0x%04X) (W: 0x1F0A0F00)\n"ANSI_COLOR_RESET, REG_RTL_MII_ETNRXCPU1);
    RegValue = HAL_MII_READ32(REG_RTL_MII_ETNRXCPU1);
    LOGD2("[B] REG_RTL_MII_ETNRXCPU1: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_ETNRXCPU1, 0x1F0A0F00);
    LOGD2("[A] REG_RTL_MII_ETNRXCPU1: %X\n", HAL_MII_READ32(REG_RTL_MII_ETNRXCPU1));

    LOGI(ANSI_COLOR_YELLOW"Rx_Pse_Des_Thres_1_h (0x%04X) (W: 0x00000022)\n"ANSI_COLOR_RESET, REG_RTL_MII_RX_PSE1);
    RegValue = HAL_MII_READ32(REG_RTL_MII_RX_PSE1);
    LOGD2("[B] REG_RTL_MII_RX_PSE1: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_RX_PSE1, 0x00000022);
    LOGD2("[A] REG_RTL_MII_RX_PSE1: %X\n", HAL_MII_READ32(REG_RTL_MII_RX_PSE1));

    LOGI(ANSI_COLOR_YELLOW"Ethernet_IO_CMD1 Register(0x%04X): Enable Rx Ring1 (W: 0x10000, BIT16)\n"ANSI_COLOR_RESET, REG_RTL_MII_IOCMD1);
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD1);
    LOGD2("[B] REG_RTL_MII_IOCMD1: %X\n", RegValue);
    RegValue |= BIT_IOCMD1_FIRST_DMARX_ENABLE(1);
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD1, RegValue);
    LOGD2("[A] REG_RTL_MII_IOCMD1: %X\n", HAL_MII_READ32(REG_RTL_MII_IOCMD1));

    LOGI(ANSI_COLOR_YELLOW"REG_RTL_MII_IOCMD Register(0x%04X): Enable Rx (W: 0x20, BIT5)\n"ANSI_COLOR_RESET, REG_RTL_MII_IOCMD);
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD);
    LOGD2("[B] REG_RTL_MII_IOCMD: %X\n", RegValue);
    RegValue |= BIT_IOCMD_RXENABLE(1);
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD, RegValue);
    LOGD2("[A] REG_RTL_MII_IOCMD: %X\n", HAL_MII_READ32(REG_RTL_MII_IOCMD));
}


VOID
HalMiiGmacSetDefaultEthIoCmdRtl8195a(
        IN VOID *Data
        )
{
    DBG_ENTRANCE;
    PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;
    PHAL_MII_ADAPTER pHalMiiAdapter = pMiiAdapter->pHalMiiAdapter;
    u32 RegValue;

    LOGI(ANSI_COLOR_YELLOW"REG_RTL_MII_IOCMD Register(0x%04X) (W: CMD_CONFIG)\n"ANSI_COLOR_RESET, REG_RTL_MII_IOCMD);
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD);
    LOGD2("[B] REG_RTL_MII_IOCMD: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD, CMD_CONFIG);
    LOGD2("[A] REG_RTL_MII_IOCMD: %X\n", HAL_MII_READ32(REG_RTL_MII_IOCMD));

    LOGI(ANSI_COLOR_YELLOW"Ethernet_IO_CMD1 Register(0x%04X) (W: CMD1_CONFIG)\n"ANSI_COLOR_RESET, REG_RTL_MII_IOCMD1);
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD1);
    LOGD2("[B] REG_RTL_MII_IOCMD1: %X\n", RegValue);
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD1, CMD1_CONFIG);
    LOGD2("[A] REG_RTL_MII_IOCMD1: %X\n", HAL_MII_READ32(REG_RTL_MII_IOCMD1));

    //2014-04-29 yclin (disable 0x40051438[27] r_en_precise_dma) {
    RegValue = HAL_MII_READ32(REG_RTL_MII_IOCMD1);
    RegValue = RegValue & 0xF7FFFFFF;
    HAL_MII_WRITE32(REG_RTL_MII_IOCMD1, RegValue);
    // }
}


VOID
HalMiiGmacInitIrqRtl8195a(
		IN VOID *Data
		)
{
	IRQ_HANDLE	MiiIrqHandle_Master;
	PMII_ADAPTER pMiiAdapter = (PMII_ADAPTER) Data;


	MiiIrqHandle_Master.Data	 = (u32) (pMiiAdapter);
	MiiIrqHandle_Master.IrqNum	 = GMAC_IRQ;
	MiiIrqHandle_Master.IrqFun	 = (IRQ_FUN) MiiIrqHandle;
	MiiIrqHandle_Master.Priority = 0;
	InterruptRegister(&MiiIrqHandle_Master);
	InterruptEn(&MiiIrqHandle_Master);
}


u32
HalMiiGmacGetInterruptStatusRtl8195a(
        VOID
        )
{
    DBG_ENTRANCE;
    u32 RegValue;

    RegValue = HAL_MII_READ32(REG_RTL_MII_IMRISR);
    LOGD("REG_RTL_MII_IMRISR: %X\n", RegValue);
    return RegValue;
}


VOID
HalMiiGmacClearInterruptStatusRtl8195a(
        u32 IsrStatus
        )
{
    DBG_ENTRANCE;
    HAL_MII_WRITE32(REG_RTL_MII_IMRISR, IsrStatus);
}


