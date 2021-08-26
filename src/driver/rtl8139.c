/*
 * Copyright 2019 Orace KPAKPO </ orace.kpakpo@yahoo.fr >
 *
 * This file is part of EOS.
 *
 * EOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <driver/rtl8139.h>
#include <io.h>
#include <pci.h>
#include <lib.h>
#define RX_BUF_LEN_IDX 0	/* 0, 1, 2 is allowed - 8,16,32K rx buffer */
#define RX_BUF_LEN (8192 << RX_BUF_LEN_IDX)
#define TX_BUF_SIZE (2048 << RX_BUF_LEN_IDX)
/* The RTL8139 can only transmit from a contiguous, aligned memory block.  */
static unsigned char __tx_buf[TX_BUF_SIZE] __attribute__((aligned(4)));
static unsigned char __rx_ring[RX_BUF_LEN + 16] __attribute__((aligned(4)));
// rtl8139 device
rtl8139_dev_t __rtl8139_dev = {0};
/* Symbolic offsets to registers. */
enum RTL8139_registers 
{
	MAC0 = 0x0,			/* Ethernet hardware address. */
	MAR0 = 0x08,			/* Multicast filter. */
	TxStatus0 = 0x10,		/* Transmit status (four 32bit registers). */
	TxAddr0 = 0x20,		/* Tx descriptors (also four 32bit). */
	RxBuf = 0x30, RxEarlyCnt = 0x34, RxEarlyStatus = 0x36,
	ChipCmd = 0x37, RxBufPtr = 0x38, RxBufAddr = 0x3A,
	IntrMask = 0x3C, IntrStatus = 0x3E,
	TxConfig = 0x40, RxConfig = 0x44,
	Timer = 0x48,		/* general-purpose counter. */
	RxMissed = 0x4C,		/* 24 bits valid, write clears. */
	Cfg9346 = 0x50, Config0 = 0x51, Config1 = 0x52,
	TimerIntrReg = 0x54,	/* intr if gp counter reaches this value */
	MediaStatus = 0x58,
	Config3 = 0x59,
	MultiIntr = 0x5C,
	RevisionID = 0x5E,	/* revision of the RTL8139 chip */
	TxSummary = 0x60,
	MII_BMCR = 0x62, MII_BMSR = 0x64, NWayAdvert = 0x66, NWayLPAR = 0x68,
	NWayExpansion = 0x6A,
	DisconnectCnt = 0x6C, FalseCarrierCnt = 0x6E,
	NWayTestReg = 0x70,
	RxCnt = 0x72,		/* packet received counter */
	CSCR = 0x74,		/* chip status and configuration register */
	PhyParm1 = 0x78, TwisterParm = 0x7c, PhyParm2 = 0x80,	/* undocumented */
	/* from 0x84 onwards are a number of power management/wakeup frame
	 * definitions we will probably never need to know about.  */
};

enum ChipCmdBits {
	CmdReset = 0x10, CmdRxEnb = 0x08, CmdTxEnb = 0x04, RxBufEmpty = 0x01, 
};
/* Interrupt register bits, using my own meaningful names. */
enum IntrStatusBits {
	PCIErr = 0x8000, PCSTimeout = 0x4000, CableLenChange = 0x2000,
	RxFIFOOver = 0x40, RxUnderrun = 0x20, RxOverflow = 0x10,
	TxErr = 0x08, TxOK = 0x04, RxErr = 0x02, RxOK = 0x01,
};
enum TxStatusBits {
	TxHostOwns = 0x2000, TxUnderrun = 0x4000, TxStatOK = 0x8000,
	TxOutOfWindow = 0x20000000, TxAborted = 0x40000000,
	TxCarrierLost = 0x80000000,
};
enum RxStatusBits {
	RxMulticast = 0x8000, RxPhysical = 0x4000, RxBroadcast = 0x2000,
	RxBadSymbol = 0x0020, RxRunt = 0x0010, RxTooLong = 0x0008, RxCRCErr = 0x0004,
	RxBadAlign = 0x0002, RxStatusOK = 0x0001,
};
enum MediaStatusBits {
	MSRTxFlowEnable = 0x80, MSRRxFlowEnable = 0x40, MSRSpeed10 = 0x08,
	MSRLinkFail = 0x04, MSRRxPauseFlag = 0x02, MSRTxPauseFlag = 0x01,
};
enum MIIBMCRBits {
	BMCRReset = 0x8000, BMCRSpeed100 = 0x2000, BMCRNWayEnable = 0x1000,
	BMCRRestartNWay = 0x0200, BMCRDuplex = 0x0100,
};
enum CSCRBits {
	CSCR_LinkOKBit = 0x0400, CSCR_LinkChangeBit = 0x0800,
	CSCR_LinkStatusBits = 0x0f000, CSCR_LinkDownOffCmd = 0x003c0,
	CSCR_LinkDownCmd = 0x0f3c0,
};
/* Bits in RxConfig. */
enum rx_mode_bits {
	RxCfgWrap = 0x80,
	AcceptErr = 0x20, AcceptRunt = 0x10, AcceptBroadcast = 0x08,
	AcceptMulticast = 0x04, AcceptMyPhys = 0x02, AcceptAllPhys = 0x01,
};

void rtl8139_init(void)
{

}

void rtl8139_handler(void)
{

}

int rtl8139_receive(void)
{

}
