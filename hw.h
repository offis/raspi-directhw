/**
 * @file
 *
 * Declarations for all publicly known (as of 08/2013) Raspberry Pi hardware
 * registers.
 *
 * License
 * -------
 *
 * Copyright (c) 2013 OFFIS e.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * @mainpage Raspberry Pi Direct Hardware Access
 *
 * This is a light-weight library for easy access to the Raspberry Pi's
 * integrated peripherals.  It can be used from user mode as well as from kernel
 * mode, and it works fine from a real-time context of one of the real-time
 * Linux patches like Xenomai.  It even works when doing bare-metal programming.
 * Note that this library does not handle the on-board LAN chip of Model B
 * RasPis.  That one is accessed through USB.
 *
 * The latest version can be obtained via GitHub: https://github.com/offis/raspi-directhw
 *
 *
 * File Structure
 * --------------
 *
 * `raspi/hw.h` contains base register declarations, most importantly @ref HW.
 * Many registers are specified down to the individual register bit.
 *
 * It also contains helper functions for GPIO and system timer access.
 * `raspi/uart.h`, `raspi/spi.h`, and `raspi/spisl.h` contain more hardware
 * helpers.
 *
 *
 * Usage
 * -----
 *
 * In order to use this API, you need to include `raspi/hw.c` in (exactly) one
 * of your source files.  There are no C files to compile, and thus no libraries
 * to link.  Everything is small enough to use inline static declarations
 * efficiently.
 *
 * If running in user mode you need to call `raspi_map_hw()` at the start of
 * your program.  After initialization, @ref HW is the single entry point to all
 * registers.
 *
 * **A final warning:** Always use `memory_barrier()` in appropriate places, no
 * matter what you use, API helpers or direct register access.
 *
 * See [Modules](modules.html) for an API overview and @ref gpio_test.c for a
 * basic code example.
 *
 *
 * License
 * -------
 *
 * Copyright (c) 2013 OFFIS e.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef RASPI_HW_H
#define RASPI_HW_H

#include <stdint.h>

/**
 * @defgroup registers Register Declarations
 *
 * The baseline feature of this library: declarations for all documented and
 * various undocumented hardware registers.
 *
 * The entries in @ref HW are named like the structs listed below, with the
 * `raspi_` prefix and `_regs` suffix removed.  @ref raspi_DMA_regs is special,
 * as it is an array. Similarly, the clock manager registers @ref raspi_CM_regs
 * is an array indexed by constants of enum @ref raspi_CM_reg_t.
 *
 * Declared in `raspi/hw.h`.
 * @{
 */

/**
 * Auxillary peripherals generic configuration. Register names have AUX_ prefix
 * stripped.
 *
 * No external signals.
 */
typedef struct {
	uint32_t IRQ;
	uint32_t ENABLES;
} raspi_AUX_regs;
/// AUX register offset
#define AUX_OFFSET 0x215000

/**
 * Auxillary mini UART (= UART1). Register names have `AUX_MU_` prefix and
 * `_REG` suffix stripped.
 *
 * Effectively unusable due to overlap with UART0, which has no usable
 * alternate mapping.
 *
 *   Signal | Mapping 1           | Mapping 2          | Mapping 3
 *  --------|---------------------|--------------------|-------------------
 *   TxD    | GPIO14 Alt5 (P1-08) | GPIO32 Alt5 (nc)   | GPIO40 Alt5 (R21)
 *   RxD    | GPIO15 Alt5 (P1-10) | GPIO33 Alt5 (nc)   | GPIO41 Alt5 (nc)
 *   CTS    | GPIO16 Alt5 (D5)    | GPIO30 Alt5 (P5-5) | GPIO43 Alt5 (nc)
 *   RTS    | GPIO17 Alt5 (P1-11) | GPIO31 Alt5 (P5-6) | GPIO42 Alt5 (nc)
 */
typedef struct {
	uint32_t IO;
	uint32_t IER;
	uint32_t IIR;
	uint32_t LCR;
	uint32_t MCR;
	uint32_t LSR;
	uint32_t MSR;
	uint32_t SCRATCH;
	uint32_t CNTL;
	uint32_t STAT;
	uint32_t BAUD;
} raspi_UART1_regs;
/// UART1 register offset
#define UART1_OFFSET 0x215040

/**
 * Auxillary mini SPI 0 (= SPI1). Register names have `AUX_SPI0_` prefix and
 * `_REG` suffix stripped.
 *
 * Unusable due to missing connections.
 *
 *   Signal | Mapping 1
 *  --------|---------------------
 *   SCLK   | GPIO21 Alt4 (S5-11)
 *   MOSI   | GPIO20 Alt4 (nc)
 *   MISO   | GPIO19 Alt4 (nc)
 *   CE0_N  | GPIO18 Alt4 (P1-12)
 *   CE1_N  | GPIO17 Alt4 (P1-11)
 *   CE2_N  | GPIO16 Alt4 (D5)
 */
typedef struct {
	uint32_t CNTL0;
	uint32_t CNTL1;
	uint32_t STAT;
	uint32_t maybe_PEEK;
	uint32_t IO;
	uint32_t PEEK; // contradiction in datasheet
	uint32_t reserved_0x18[2];
	uint32_t maybe_IO[4];
	uint32_t maybe_TXHOLD[4];
} raspi_SPI1_regs;
/// SPI1 register offset
#define SPI1_OFFSET 0x215080


/**
 * Auxillary mini SPI1 (= SPI2). Register names have `AUX_SPI1_` prefix and
 * `_REG` suffix stripped.
 *
 * Unusable due to missing connections.
 *
 *   Signal | Mapping 1
 *  --------|---------------------
 *   SCLK   | GPIO42 Alt4 (nc)
 *   MOSI   | GPIO41 Alt4 (nc)
 *   MISO   | GPIO40 Alt4 (R21)
 *   CE0_N  | GPIO43 Alt4 (nc)
 *   CE1_N  | GPIO44 Alt4 (nc)
 *   CE2_N  | GPIO45 Alt4 (R27)
 */
typedef raspi_SPI1_regs raspi_SPI2_regs;
/// SPI2 register offset
#define SPI2_OFFSET 0x2150c0


/**
 * BSC0 (I2C0) master.
 *
 * Standard mapping: S5-13/S5-14.
 *
 *   Signal | Mapping 1           | Mapping 2          | Mapping 3
 *  --------|---------------------|--------------------|-------------------
 *   SDA    | GPIO0 Alt0 (S5-14)  | GPIO28 Alt0 (P5-3) | GPIO44 Alt1 (nc)
 *   SCL    | GPIO1 Alt0 (S5-13)  | GPIO29 Alt0 (P5-4) | GPIO45 Alt1 (R27)
 */
typedef struct {
	uint32_t C;
	uint32_t S;
	uint32_t DLEN;
	uint32_t A;
	uint32_t FIFO;
	uint32_t DIV;
	uint32_t DEL;
	uint32_t CLKT;
} raspi_BSC0_regs;
/// BSC0 register offset
#define BSC0_OFFSET 0x205000


/**
 * BSC1 (I2C1) master.
 *
 * Standard mapping: P1-03/P1-05.
 *
 *   Signal | Mapping 1           | Mapping 2
 *  --------|---------------------|--------------------
 *   SDA    | GPIO2 Alt0 (P1-03)  | GPIO44 Alt2 (nc)
 *   SCL    | GPIO3 Alt0 (P1-05)  | GPIO45 Alt2 (R27)
 */
typedef raspi_BSC0_regs raspi_BSC1_regs;
/// BSC1 register offset
#define BSC1_OFFSET 0x804000


/**
 * BSC2 (I2C2) master.
 *
 * Unusable due to being part of the HDMI interface.
 */
typedef raspi_BSC0_regs raspi_BSC2_regs;
/// BSC2 register offset
#define BSC2_OFFSET 0x805000


/**
 * DMA control block.
 *
 * This data structure is expected at the address written to CONBLK_AD /
 * NEXTCONBK.
 */
typedef struct {
	uint32_t TI;
	uint32_t SOURCE_AD;
	uint32_t DEST_AD;
	uint32_t TXFR_LEN;
	uint32_t STRIDE;
	uint32_t NEXTCONBK;
	uint32_t DEBUG; // actually a DMA engine register; ignored in the CB
	uint32_t reserved_0x1c;
} raspi_dma_control_block;


/**
 * DMA channel 15.
 *
 * No external signals.
 */
typedef struct {
	uint32_t CS;
	uint32_t CONBLK_AD;
	raspi_dma_control_block CB;
	uint32_t reserved_0x28[54];
} raspi_DMA15_regs;
/// DMA 15 register offset
#define DMA15_OFFSET 0xe05000


/**
 * DMA channels 0-14.
 *
 * No external signals.
 */
typedef raspi_DMA15_regs raspi_DMA_regs[15];
/// DMA 0-14 register offset
#define DMA_OFFSET 0x007000


/**
 * DMA global control.
 *
 * No external signals.
 */
typedef struct {
	uint32_t INT_STATUS;
	uint32_t reserved_0x04[3];
	uint32_t ENABLE;
} raspi_DMA_GLOBAL_regs;
/// DMA_GLOBAL register offset
#define DMA_GLOBAL_OFFSET 0x007fe0


/**
 * Hardware Random Number Generator.
 *
 * No external signals.
 */
typedef struct {
	union {
		uint32_t U;
		struct raspi_RNG_CTRL_reg {
			// Generate random numbers. Enabled after reset.
			uint32_t RBGEN:1;
			// Double speed RNG. Reduces randomness.
			uint32_t RBG2X:1;
			uint32_t reserved:30;
		} B;
	} CTRL;
	union {
		uint32_t U;
		struct raspi_RNG_STATUS_reg {
			// Warm up count.
			uint32_t WARM_CNT:20;
			uint32_t reserved:4;
			// Number of valid words in FIFO.
			uint32_t VAL:8;
		} B;
	} STATUS;
	uint32_t DATA;
	union {
		uint32_t U;
		struct raspi_RNG_FF_THRES_reg {
			// Assert interrupt at this many words in FIFO.
			uint32_t FF_THRESH:8;
			uint32_t reserved:24;
		} B;
	} FF_THRES;
	union {
		uint32_t U;
		struct raspi_RNG_INT_MASK_reg {
			// Mask FIFO interrupt.
			uint32_t INT_OFF:1;
			uint32_t reserved:31;
		} B;
	} INT_MASK;
} raspi_RNG_regs;
/// RNG register offset
#define RNG_OFFSET 0x104000


/**
 * External Mass Media Controller (MMC/SD/SDIO).
 *
 * Standard mapping: SD-Card connector.
 *
 *   Signal   | Mapping 1         | Mapping 2
 *  ----------|-------------------|----------------------
 *   CARD_DET | GPIO46 Alt0? (SD) | none
 *   CLK      | GPIO47 Alt0? (SD) | GPIO22 Alt3? (P1-15)
 *   CMD      | GPIO48 Alt0? (SD) | GPIO23 Alt3? (P1-16)
 *   DATA0    | GPIO49 Alt0? (SD) | GPIO24 Alt3? (P1-18)
 *   DATA1    | GPIO50 Alt0? (SD) | GPIO25 Alt3? (P1-22)
 *   DATA2    | GPIO51 Alt0? (SD) | GPIO26 Alt3? (nc)
 *   DATA3    | GPIO52 Alt0? (SD) | GPIO27 Alt3? (P1-13)
 */
typedef struct {
	uint32_t ARG2;
	uint32_t BLKSIZECNT;
	uint32_t ARG1;
	uint32_t CMDTM;
	uint32_t RESP[4];
	uint32_t DATA;
	uint32_t STATUS;
	uint32_t CONTROL0;
	uint32_t CONTROL1;
	uint32_t INTERRUPT;
	uint32_t IRPT_MASK;
	uint32_t IRPT_EN;
	uint32_t CONTROL2;
	uint32_t reserved_0x40[4];
	uint32_t FORCE_IRPT;
	uint32_t reserved_0x54[7];
	uint32_t BOOT_TIMEOUT;
	uint32_t DBG_SEL;
	uint32_t reserved_0x78[2];
	uint32_t EXRDFIFO_CFG;
	uint32_t EXRDFIFO_EN;
	uint32_t TUNE_STEP;
	uint32_t TUNE_STEP_STD;
	uint32_t TUNE_STEP_DDR;
	uint32_t reserved_0x94[19];
	uint32_t SPI_INT_SPT;
	uint32_t reserved_0xf4[2];
	uint32_t SLOTISR_VER;
} raspi_EMMC_regs;
/// EMMC register offset
#define EMMC_OFFSET 0x300000


/**
 * General-Purpose I/O.
 *
 * P1 pin mapping (main GPIO connector):
 *
 *     3.3V | GPIO2 | GPIO3 | GPIO4      | GND    | GPIO17 | GPIO27 | GPIO22 | 3.3V   | GPIO10 | GPIO9  | GPIO11 | GND
 *          | SDA1  | SCL1  | GPCLK0/TDI |        |        | TMS    | TRST   |        | MOSI   | MISO   | SCK    |
 *     -------------------------------------------------------------------------------------------------------------------
 *     5V   | 5V    | GND   | GPIO14     | GPIO15 | GPIO18 | GND    | GPIO23 | GPIO24 | GND    | GPIO25 | GPIO8  | GPIO7
 *          |       |       | TxD        | RxD    | PWM0   |        | RTCK   | TDO    | TCK    |        | CE0_N  | CE1_N
 *
 * P5 pin mapping (PCM):
 *
 *     5V   | GPIO28 | GPIO30 | GND
 *          | CLK    | DIN    |
 *     --------------------------------
 *     3.3V | GPIO29 | GPIO31 | GND
 *          | FS     | DOUT   |
 *
 * S5 pin mapping (CSI):
 *
 *   GPIO   | Pin   | Default Function
 *  --------|-------|-------------------
 *   GPIO21 | S5-11 | GPIO
 *   GPIO5  | S5-12 | GPCLK1
 *   GPIO1  | S5-13 | SCL0
 *   GPIO0  | S5-14 | SDA0
 *
 * S6 pin mapping (3.5mm audio):
 *
 *   GPIO   | Pin                         | Default Function
 *  --------|-----------------------------|-------------------
 *   GPIO40 | right channel via RC filter | PWM0
 *   GPIO45 | left channel via RC filter  | PWM1
 *
 * S8 pin mapping (SD):
 *
 *   GPIO   | Pin         | Connection
 *  --------|-------------|-------------------
 *   GPIO47 | Card Detect | direct
 *   GPIO48 | Clock       | via 33R
 *   GPIO49 | Command     | via 33R
 *   GPIO50 | Data 0      | via 33R
 *   GPIO51 | Data 1      | via 33R
 *   GPIO52 | Data 2      | via 33R
 *   GPIO53 | Data 3      | via 33R
 *
 * Other GPIOs:
 *
 *   GPIO   | Pin
 *  --------|--------
 *   GPIO6  | IC3-12
 *   GPIO16 | D5
 *   GPIO46 | IC1-6
 *
 * Not connected: 12, 13, 19, 20, 26, 32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44
 */
typedef struct {
	uint32_t FSEL[6];
	uint32_t reserved_0x18;
	uint32_t SET[2];
	uint32_t reserved_0x24;
	uint32_t CLR[2];
	uint32_t reserved_0x30;
	uint32_t LEV[2];
	uint32_t reserved_0x3c;
	uint32_t EDS[2];
	uint32_t reserved_0x48;
	uint32_t REN[2];
	uint32_t reserved_0x54;
	uint32_t FEN[2];
	uint32_t reserved_0x60;
	uint32_t HEN[2];
	uint32_t reserved_0x6c;
	uint32_t LEN[2];
	uint32_t reserved_0x78;
	uint32_t AREN[2];
	uint32_t reserved_0x84;
	uint32_t AFEN[2];
	uint32_t reserved_0x90;
	uint32_t PUD;
	uint32_t PUDCLK[2];
	uint32_t reserved_0xa0[4];
	uint32_t Test;
} raspi_GPIO_regs;
/// GPIO register offset
#define GPIO_OFFSET 0x200000


/**
 * General Purpose Clock.
 *
 * Standard mapping: GPCLK0 = P1-7, GPCLK1 = S5-12, GPCLK2 = unusable
 *
 *   Signal | Mapping 1           | Mapping 2        | Mapping 3        | Mapping 4
 *  --------|---------------------|------------------|------------------|-----------------
 *   GPCLK0 | GPIO4 Alt0 (P1-7)   | GPIO20 Alt5 (nc) | GPIO32 Alt0 (nc) | GPIO34 Alt0 (nc)
 *   GPCLK1 | GPIO5 Alt0 (S5-12)  | GPIO21 Alt5 (nc) | GPIO42 Alt0 (nc) | GPIO44 Alt0 (nc)
 *   GPCLK2 | GPIO6 Alt0 (IC3-12) | GPIO43 Alt0 (nc) |                  |
 *
 * Note that there is no GPCLK entry in @ref HW.  Use `HW.CM[GP0]` (or
 * `[GP1]`/`[GP2]`) instead.
 */
typedef struct {
	struct {
		uint32_t CTL;
		uint32_t DIV;
	} CM[3];
} raspi_GPCLK_regs;
/// GPCLK register offset
#define GPCLK_OFFSET 0x101070


/**
 * Interrupt Controller.
 *
 * No external signals.
 */
typedef struct {
	uint32_t pending_basic;
	uint32_t pending[2];
	uint32_t FIQ;
	uint32_t enable[2];
	uint32_t enable_basic;
	uint32_t disable[2];
	uint32_t disable_basic;
} raspi_IRQ_regs;
/// IRQ register offset
#define IRQ_OFFSET 0x00b200


/**
 * PCM / I2S Audio.
 *
 * Standard mapping: P5
 *
 *   Signal | Mapping 1           | Mapping 2
 *  --------|---------------------|--------------------
 *   CLK    | GPIO18 Alt0 (P1-12) | GPIO28 Alt2 (P5-3)
 *   FS     | GPIO19 Alt0 (nc)    | GPIO29 Alt2 (P5-4)
 *   DIN    | GPIO20 Alt0 (nc)    | GPIO30 Alt2 (P5-5)
 *   DOUT   | GPIO21 Alt0 (S5-11) | GPIO31 Alt2 (P5-6)
 */
typedef struct {
	union {
		uint32_t U;
		struct raspi_PCM_CS_reg {
			uint32_t EN:1;
			uint32_t RXON:1;
			uint32_t TXON:1;
			uint32_t TXCLR:1;
			uint32_t RXCLR:1;
			uint32_t TXTHR:2;
			uint32_t RXTHR:2;
			uint32_t DMAEN:1;
			uint32_t reserved0:3;
			uint32_t TXSYNC:1;
			uint32_t RXSYNC:1;
			uint32_t TXERR:1;
			uint32_t RXERR:1;
			uint32_t TXW:1;
			uint32_t RXR:1;
			uint32_t TXD:1;
			uint32_t RXD:1;
			uint32_t TXE:1;
			uint32_t RXF:1;
			uint32_t RXSEX:1;
			uint32_t SYNC:1;
			uint32_t STBY:1;
			uint32_t reserved1:6;
		} B;
	} CS;
	uint32_t FIFO;
	union {
		uint32_t U;
		struct raspi_PCM_MODE_reg {
			uint32_t FSLEN:10;
			uint32_t FLEN:10;
			uint32_t FSI:1;
			uint32_t FSM:1;
			uint32_t CLKI:1;
			uint32_t CLKM:1;
			uint32_t FTXP:1;
			uint32_t FRXP:1;
			uint32_t PDME:1;
			uint32_t PDMN:1;
			uint32_t CLK_DIS:1;
			uint32_t reserved:3;
		} B;
	} MODE;
	union {
		uint32_t U;
		struct raspi_PCM_RXC_reg {
			uint32_t CH2WID:4;
			uint32_t CH2POS:10;
			uint32_t CH2EN:1;
			uint32_t CH2WEX:1;
			uint32_t CH1WID:4;
			uint32_t CH1POS:10;
			uint32_t CH1EN:1;
			uint32_t CH1WEX:1;
		} B;
	} RXC;
	union {
		uint32_t U;
		struct raspi_PCM_TXC_reg {
			uint32_t CH2WID:4;
			uint32_t CH2POS:10;
			uint32_t CH2EN:1;
			uint32_t CH2WEX:1;
			uint32_t CH1WID:4;
			uint32_t CH1POS:10;
			uint32_t CH1EN:1;
			uint32_t CH1WEX:1;
		} B;
	} TXC;
	union {
		uint32_t U;
		struct raspi_PCM_DREQ_reg {
			uint32_t RX:7;
			uint32_t reserved0:1;
			uint32_t TX:7;
			uint32_t reserved1:1;
			uint32_t RX_PANIC:7;
			uint32_t reserved2:1;
			uint32_t TX_PANIC:7;
			uint32_t reserved3:1;
		} B;
	} DREQ;
	union {
		uint32_t U;
		struct raspi_PCM_INTEN_reg {
			uint32_t TXW:1;
			uint32_t RXR:1;
			uint32_t TXERR:1;
			uint32_t RXERR:1;
			uint32_t reserved:28;
		} B;
	} INTEN;
	union {
		uint32_t U;
		struct raspi_PCM_INTSTC_reg {
			uint32_t TXW:1;
			uint32_t RXR:1;
			uint32_t TXERR:1;
			uint32_t RXERR:1;
			uint32_t reserved:28;
		} B;
	} INTSTC;
	union {
		uint32_t U;
		struct raspi_PCM_GRAY_reg {
			uint32_t EN:1;
			uint32_t CLR:1;
			uint32_t FLUSH:1;
			uint32_t reserved0:1;
			uint32_t RXLEVEL:6;
			uint32_t FLUSHED:6;
			uint32_t RXFIFOLEVEL:6;
			uint32_t reserved1:10;
		} B;
	} GRAY;
} raspi_PCM_regs;
/// PCM register offset
#define PCM_OFFSET 0x203000


/**
 * Pulse Width Modulator.
 *
 * Standard mapping: S6 via RC-filter.
 *
 *   Signal | Mapping 1        | Mapping 2           | Mapping 3          | Mapping 4
 *  --------|------------------|---------------------|--------------------|--------------------
 *   PWM0   | GPIO12 Alt0 (nc) | GPIO18 Alt5 (P1-12) | GPIO40 Alt0 (S6-R) |
 *   PWM1   | GPIO13 Alt0 (nc) | GPIO19 Alt5 (nc)    | GPIO41 Alt0 (nc)   | GPIO45 Alt0 (S6-L)
 */
typedef struct {
	uint32_t CTL;
	uint32_t STA;
	uint32_t DMAC;
	uint32_t RNG1;
	uint32_t DAT1;
	uint32_t FIF1;
	uint32_t RNG2;
	uint32_t DAT2;
} raspi_PWM_regs;
/// PWM register offset
#define PWM_OFFSET 0x20c000


/**
 * Serial Peripheral Interface master (SPI0).
 *
 * Standard mapping: P1.
 *
 *   Signal | Mapping 1           | Mapping 2
 *  --------|---------------------|------------------
 *   SCLK   | GPIO11 Alt0 (P1-23) | GPIO39 Alt0 (nc)
 *   MOSI   | GPIO10 Alt0 (P1-19) | GPIO38 Alt0 (nc)
 *   MISO   | GPIO9 Alt0 (P1-21)  | GPIO37 Alt0 (nc)
 *   CE0_N  | GPIO8 Alt0 (P1-24)  | GPIO36 Alt0 (nc)
 *   CE1_N  | GPIO7 Alt0 (P1-26)  | GPIO35 Alt0 (nc)
 */
typedef struct {
	union {
		uint32_t U;
		struct raspi_SPI0_CS_reg {
			uint32_t CS:2;
			uint32_t CPHA:1;
			uint32_t CPOL:1;
			uint32_t CLEAR:2;
			uint32_t CSPOL:1;
			uint32_t TA:1;
			uint32_t DMAEN:1;
			uint32_t INTD:1;
			uint32_t INTR:1;
			uint32_t ADCS:1;
			uint32_t REN:1;
			uint32_t LEN:1;
			uint32_t LMONO:1;
			uint32_t TE_EN:1;
			uint32_t DONE:1;
			uint32_t RXD:1;
			uint32_t TXD:1;
			uint32_t RXR:1;
			uint32_t RXF:1;
			uint32_t CSPOL0:1;
			uint32_t CSPOL1:1;
			uint32_t CSPOL2:1;
			uint32_t DMA_LEN:1;
			uint32_t LEN_LONG:1;
			uint32_t reserved:6;
		} B;
	} CS;
	uint32_t FIFO;
	union {
		uint32_t U;
		struct raspi_SPI0_CLK_reg {
			uint32_t CDIV:16;
			uint32_t reserved:16;
		} B;
	} CLK;
	union {
		uint32_t U;
		struct raspi_SPI0_DLEN_reg {
			uint32_t LEN:16;
			uint32_t reserved:16;
		} B;
	} DLEN;
	union {
		uint32_t U;
		struct raspi_SPI0_LTOH_reg {
			uint32_t TOH:16;
			uint32_t reserved:16;
		} B;
	} LTOH;
	union {
		uint32_t U;
		struct raspi_SPI0_DC_reg {
			uint32_t TDREQ:8;
			uint32_t TPANIC:8;
			uint32_t RDREQ:8;
			uint32_t RDPANIC:8;
		} B;
	} DC;
} raspi_SPI0_regs;
/// SPI0 register offset
#define SPI0_OFFSET 0x204000


/**
 * BSC/SPI slave.
 *
 * Unusable due to missing connections.
 *
 *   Signal   | Mapping 1
 *  ----------|---------------------
 *   SCLK/SCL | GPIO19 Alt3 (nc)
 *   MOSI/SDA | GPIO18 Alt3 (P1-12)
 *   MISO     | GPIO20 Alt3 (nc)
 *   CE_N     | GPIO21 Alt3 (S5-11)
 */
typedef struct {
	uint32_t DR;
	uint32_t RSR;
	uint32_t SLV;
	uint32_t CR;
	uint32_t FR;
	uint32_t IFLS;
	uint32_t IMSC;
	uint32_t RIS;
	uint32_t MIS;
	uint32_t ICR;
	uint32_t DMACR;
	uint32_t TDR;
	uint32_t GPUSTAT;
	uint32_t HCTRL;
	uint32_t DEBUG1;
	uint32_t DEBUG2;
} raspi_BSCSL_regs;
/// BSCSL register offset
#define BSCSL_OFFSET 0x214000


/**
 * System Timer.
 *
 * No external signals.
 */
typedef struct {
	union {
		uint32_t U;
		struct raspi_ST_CS_reg {
			uint32_t M0:1;
			uint32_t M1:1;
			uint32_t M2:1;
			uint32_t M3:1;
			uint32_t reserved:28;
		} B;
	} CS;
	uint32_t CLO;
	uint32_t CHI;
	uint32_t C[4];
} raspi_ST_regs;
/// ST register offset
#define ST_OFFSET 0x003000


/**
 * ARM UART (UART0).
 *
 * Standard mapping: P1, no handshake.
 *
 *   Signal | Mapping 1           | Mapping 2          | Mapping 3
 *  --------|---------------------|--------------------|-------------------
 *   TxD    | GPIO14 Alt0 (P1-08) | GPIO32 Alt3 (nc)   | GPIO36 Alt2 (nc)
 *   RxD    | GPIO15 Alt0 (P1-10) | GPIO33 Alt3 (nc)   | GPIO37 Alt2 (nc)
 *   CTS    | GPIO16 Alt3 (D5)    | GPIO30 Alt3 (P5-5) | GPIO39 Alt2 (nc)
 *   RTS    | GPIO17 Alt3 (P1-11) | GPIO31 Alt3 (P5-6) | GPIO38 Alt2 (nc)
 */
typedef struct {
	union {
		uint32_t U;
		struct raspi_UART0_DR_reg {
			uint32_t DATA:8;
			uint32_t FE:1;
			uint32_t PE:1;
			uint32_t BE:1;
			uint32_t OE:1;
			uint32_t reserved:20;
		} B;
	} DR;
	union {
		uint32_t U;
		struct raspi_UART0_RSRECR_reg {
			uint32_t FE:1;
			uint32_t PE:1;
			uint32_t BE:1;
			uint32_t OE:1;
			uint32_t reserved:28;
		} B;
	} RSRECR;
	uint32_t reserved_0x08[4];
	union {
		uint32_t U;
		struct raspi_UART0_FR_reg {
			uint32_t CTS:1;
			uint32_t DSR:1;
			uint32_t DCD:1;
			uint32_t BUSY:1;
			uint32_t RXFE:1;
			uint32_t TXFF:1;
			uint32_t RXFF:1;
			uint32_t TXFE:1;
			uint32_t RI:1;
			uint32_t reserved:23;
		} B;
	} FR;
	uint32_t reserved_0x1c;
	uint32_t ILPR; // disabled
	union {
		uint32_t U;
		struct raspi_UART0_IBRD_reg {
			uint32_t IBRD:16;
			uint32_t reserved:16;
		} B;
	} IBRD;
	union {
		uint32_t U;
		struct raspi_UART0_FBRD_reg {
			uint32_t FBRD:6;
			uint32_t reserved:26;
		} B;
	} FBRD;
	union {
		uint32_t U;
		struct raspi_UART0_LCRH_reg {
			uint32_t BRK:1;
			uint32_t PEN:1;
			uint32_t EPS:1;
			uint32_t STP2:1;
			uint32_t FEN:1;
			uint32_t WLEN:2;
			uint32_t SPS:1;
			uint32_t reserved:24;
		} B;
	} LCRH;
	union {
		uint32_t U;
		struct raspi_UART0_CR_reg {
			uint32_t UARTEN:1;
			uint32_t SIREN:1;
			uint32_t SIRLP:1;
			uint32_t reserved0:4;
			uint32_t LBE:1;
			uint32_t TXE:1;
			uint32_t RXE:1;
			uint32_t DTR:1;
			uint32_t RTS:1;
			uint32_t OUT1:1;
			uint32_t OUT2:1;
			uint32_t RTSEN:1;
			uint32_t CTSEN:1;
			uint32_t reserved1:16;
		} B;
	} CR;
	union {
		uint32_t U;
		struct raspi_UART0_IFLS_reg {
			uint32_t TXIFLSEL:3;
			uint32_t RXIFLSEL:3;
			uint32_t TXIFPSEL:3;
			uint32_t RXIFPSEL:3;
			uint32_t reserved:20;
		} B;
	} IFLS;
	union {
		uint32_t U;
		struct raspi_UART0_IMSC_reg {
			uint32_t RIMM:1;
			uint32_t CTSMIM:1;
			uint32_t DCDMIM:1;
			uint32_t DSRMIM:1;
			uint32_t RXIM:1;
			uint32_t TXIM:1;
			uint32_t RTIM:1;
			uint32_t FEIM:1;
			uint32_t PEIM:1;
			uint32_t BEIM:1;
			uint32_t OEIM:1;
			uint32_t reserved:21;
		} B;
	} IMSC;
	union {
		uint32_t U;
		struct raspi_UART0_RIS_reg {
			uint32_t RIRMIS:1;
			uint32_t CTSRMIS:1;
			uint32_t DCDRMIS:1;
			uint32_t DSRRMIS:1;
			uint32_t RXRIS:1;
			uint32_t TXRIS:1;
			uint32_t RTRIS:1;
			uint32_t FERIS:1;
			uint32_t PERIS:1;
			uint32_t BERIS:1;
			uint32_t OERIS:1;
			uint32_t reserved:21;
		} B;
	} RIS;
	union {
		uint32_t U;
		struct raspi_UART0_MIS_reg {
			uint32_t RIMMIS:1;
			uint32_t CTSMMIS:1;
			uint32_t DCDMMIS:1;
			uint32_t DSRMMIS:1;
			uint32_t RXMIS:1;
			uint32_t TXMIS:1;
			uint32_t RTMIS:1;
			uint32_t FEMIS:1;
			uint32_t PEMIS:1;
			uint32_t BEMIS:1;
			uint32_t OEMIS:1;
			uint32_t reserved:21;
		} B;
	} MIS;
	union {
		uint32_t U;
		struct raspi_UART0_ICR_reg {
			uint32_t RIMIC:1;
			uint32_t CTSMIC:1;
			uint32_t DCDMIC:1;
			uint32_t DSRMIC:1;
			uint32_t RXIC:1;
			uint32_t TXIC:1;
			uint32_t RTIC:1;
			uint32_t FEIC:1;
			uint32_t PEIC:1;
			uint32_t BEIC:1;
			uint32_t OEIC:1;
			uint32_t reserved:21;
		} B;
	} ICR;
	uint32_t DMACR; // disabled
	uint32_t reserved_0x4c[14];
	union {
		uint32_t U;
		struct raspi_UART0_ITCR_reg {
			uint32_t ITCR0:1;
			uint32_t ITCR1:1;
			uint32_t reserved:30;
		} B;
	} ITCR;
	union {
		uint32_t U;
		struct raspi_UART0_ITIP_reg {
			uint32_t ITIP0:1;
			uint32_t reserved0:2;
			uint32_t ITIP3:1;
			uint32_t reserved1:28;
		} B;
	} ITIP;
	union {
		uint32_t U;
		struct raspi_UART0_ITOP_reg {
			uint32_t ITOP0:1;
			uint32_t reserved0:2;
			uint32_t ITOP3:1;
			uint32_t reserved1:2;
			uint32_t ITOP6:1;
			uint32_t ITOP7:1;
			uint32_t ITOP8:1;
			uint32_t ITOP9:1;
			uint32_t ITOP10:1;
			uint32_t ITOP11:1;
			uint32_t reserved2:20;
		} B;
	} ITOP;
	union {
		uint32_t U;
		struct raspi_UART0_TDR_reg {
			uint32_t TDR10_0:11;
			uint32_t reserved0:21;
		} B;
	} TDR;
} raspi_UART0_regs;
/// UART0 register offset
#define UART0_OFFSET 0x201000


/**
 * ARM Timer.
 *
 * No external signals.
 */
typedef struct {
	uint32_t Load;
	uint32_t Value;
	uint32_t Control;
	uint32_t IRQClearAck;
	uint32_t RawIRQ;
	uint32_t MaskedIRQ;
	uint32_t Reload;
	uint32_t Predivider;
	uint32_t Freerunning;
} raspi_TIMER_regs;
/// TIMER register offset
#define TIMER_OFFSET 0x00b400


/**
 * USB controller.
 *
 * Uses dedicated pins connected to USB/LAN chip.
 */
typedef struct {
	uint32_t unknown[32];
	uint32_t MDIO_CNTL;
	uint32_t MDIO_GEN;
	uint32_t VBUS_DRV;
} raspi_USB_regs;
/// USB register offset
#define USB_OFFSET 0x980000


/**
 * Legacy MMC Controller.
 *
 * Supposedly connected to the same pins as the EMMC.
 */
typedef struct {
	uint32_t Command;
	uint32_t Argument;
	uint32_t Timeout;
	uint32_t ClkDiv;
	uint32_t Response[4];
	uint32_t Status;
	uint32_t Unknown_0x24[3];
	uint32_t VDD;
	uint32_t EDM;
	uint32_t HostConfig;
	uint32_t HBCT;
	uint32_t Data;
	uint32_t Unknown_0x44[3];
	uint32_t HBLC;
} raspi_MMC_regs;
/// MMC register offset
#define MMC_OFFSET 0x202000


/**
 * Clock Management.
 *
 * No external connections.
 */
typedef struct {
	union {
		uint32_t U;
		struct raspi_CM_CTL_reg {
			uint32_t SRC:4;
			uint32_t ENAB:1;
			uint32_t KILL:1;
			uint32_t reserved0:1;
			uint32_t BUSY:1;
			uint32_t FLIP:1;
			uint32_t MASH:2;
			uint32_t reserved1:13;
			uint32_t PASSWD:8;
		} B;
	} CTL;
	union {
		uint32_t U;
		struct raspi_CM_DIV_reg {
			uint32_t DIVF:12;
			uint32_t DIVI:12;
			uint32_t PASSWD:8;
		} B;
	} DIV;
} raspi_CM_reg;
/// Clock manager sources.
typedef enum {
	CM_GND = 0,
	CM_OSC = 1,
	CM_PLLA = 4,
	CM_PLLC = 5,
	CM_PLLD = 6,
	CM_HDMI = 7
} raspi_CM_CTL_SRC_t;
/// Known clock manager entries.
typedef enum {
	CM_VPU = 1,
	CM_H264 = 5,
	CM_UNK_0x30 = 6,
	CM_V3D = 7,
	CM_CAM0_LP = 8,
	CM_DSI_ESC = 11,
	CM_DPI = 13,
	CM_GP0 = 14,
	CM_GP1 = 15,
	CM_GP2 = 16,
	CM_HSM = 17,
	CM_ISP = 18,
	CM_PCM = 19,
	CM_PWM = 20,
	CM_SLIM = 21,
	CM_SMI = 22,
	CM_EMMC = 24,
	CM_TSENS = 28,
	CM_TIME = 29,
	CM_UART = 30,
	CM_VEC = 31,
	CM_UNK_0x190 = 50,
	CM_ARM = 54,
	CM_UNK_0x1C0 = 56
} raspi_CM_reg_t;
/// Password for the PASSWD field of various Clock Manager registers.
#define CM_PASSWD (0x5a)
/// Clock Manager register array.
typedef raspi_CM_reg raspi_CM_regs[57];
/// CM register offset
#define CM_OFFSET 0x101000


/**
 * Power/Reset Management.
 *
 * No external connections.
 */
typedef struct {
	uint32_t unknown_0x00[0x1c];
	union {
		uint32_t U;
		struct raspi_PM_RSTC_reg {
			uint32_t reserved0:1;
			uint32_t RESET1:1;
			uint32_t reserved1:2;
			uint32_t WRCFG:2;
			uint32_t reserved2:2;
			uint32_t RESET2:1;
			uint32_t reserved3:15;
			uint32_t PASSWD:8;
		} B;
	} RSTC;
	union {
		uint32_t U;
		struct raspi_PM_RSTS_reg {
			uint32_t HADDRQ:1;
			uint32_t HADDRF:1;
			uint32_t HADDRH:1;
			uint32_t reserved0:1;
			uint32_t HADWRQ:1;
			uint32_t HADWRF:1;
			uint32_t HADWRH:1;
			uint32_t reserved1:1;
			uint32_t HADDSRQ:1;
			uint32_t HADDSRF:1;
			uint32_t HADDSRH:1;
			uint32_t reserved2:1;
			uint32_t HADPOR:1;
			uint32_t reserved3:19;
		} B;
	} RSTS;
	union {
		uint32_t U;
		struct raspi_PM_WDOG_reg {
			uint32_t TIME_SET:20;
			uint32_t reserved:4;
			uint32_t PASSWD:8;
		} B;
	} WDOG;
} raspi_PM_regs;
/// Possible values for [`HW.PM.RSTC.B.WRCFG`](@ref raspi_PM_regs).
typedef enum {
	PM_CLR = 0,
	PM_SET = 1,
	PM_FULL_RESET = 2
} raspi_PM_RSTC_WRCFG_t;
/// Password for the PASSWD field of various Power Management registers.
#define PM_PASSWD (0x5a)
/// PM register offset
#define PM_OFFSET 0x100000


/// @}

#define reg(src) raspi_##src##_regs src;
#define pad(src,dest) uint8_t reserved_##src##_##dest[((dest##_OFFSET)-(src##_OFFSET)-sizeof(raspi_##src##_regs))]
#define HW_END_OFFSET 0xf00000

/**
 * Contains all perpherals declared in this file. See @ref HW for suggested
 * usage.
 *
 * Documentation taken from the official data sheet and
 * https://github.com/hermanhermitage/videocoreiv/wiki/Register-Documentation
 */
typedef volatile struct {
	/// \cond
	uint8_t reserved_BEGIN[ST_OFFSET];
	/// \endcond
	                        // 0x000000 Multicore Sync
	                        // 0x001000 Compact Camera Port 2 TX
	                        // 0x002000 Videocore Interrupt Controller
	reg(ST);                // 0x003000
	pad(ST, DMA);           // 0x004000 " txp"
	                        // 0x005000 "jpeg"
	                        // 0x006000 Message-based Parallel Host Interface
	reg(DMA);               // 0x007000
	pad(DMA, DMA_GLOBAL);
	reg(DMA_GLOBAL);
	pad(DMA_GLOBAL, IRQ);   // 0x009000 "ARBA"
	                        // 0x00a000 "brdg"
	reg(IRQ);               // 0x00b200
	pad(IRQ, TIMER);
	reg(TIMER);             // 0x00b400
	pad(TIMER, PM);
	reg(PM);                // 0x100000
	pad(PM, CM);
	reg(CM);                // 0x101000 (includes GPCLK at 0x101070)
	pad(CM, RNG);           // 0x102000 "a2w"
	                        // 0x103000 Audio Video Standard
	reg(RNG);               // 0x104000
	pad(RNG, GPIO);
	reg(GPIO);              // 0x200000
	pad(GPIO, UART0);
	reg(UART0);             // 0x201000
	pad(UART0, MMC);
	reg(MMC);               // 0x202000
	pad(MMC, PCM);
	reg(PCM);               // 0x203000
	pad(PCM, SPI0);
	reg(SPI0);              // 0x204000
	pad(SPI0, BSC0);
	reg(BSC0);              // 0x205000
	pad(BSC0, PWM);         // 0x206000 "pixv"
	                        // 0x207000 "pixv"
	                        // 0x208000 "dpi"
	                        // 0x209000 Display Serial Interface
	reg(PWM);               // 0x20c000
	pad(PWM, BSCSL);        // 0x20d000 "perm"
	                        // 0x20e000 "tec"
	                        // 0x20f000 "otp0"
	                        // 0x210000 SLIMbus
	                        // 0x211000 "cpg"
	                        // 0x212000 ?
	                        // 0x213000 "AVSP"
	reg(BSCSL);             // 0x214000
	pad(BSCSL, AUX);
	reg(AUX);               // 0x215000
	pad(AUX, UART1);
	reg(UART1);             // 0x215040
	pad(UART1, SPI1);
	reg(SPI1);              // 0x215080
	pad(SPI1, SPI2);
	reg(SPI2);              // 0x2150c0
	pad(SPI2, EMMC);
	reg(EMMC);              // 0x300000
	pad(EMMC, BSC1);        // 0x400000 ddrv
	                        // 0x600000 SMI
	                        // 0x800000 "ucam"
	                        // 0x802000 "cmi"
	reg(BSC1);              // 0x804000
	pad(BSC1, BSC2);
	reg(BSC2);              // 0x805000
	pad(BSC2, USB);         // 0x806000 "veca"
	                        // 0x807000 "pixv"
	                        // 0x808000 "hdmi"
	                        // 0x809000 "hdcp"
	                        // 0x80a000 "arbr"
	                        // 0x900000 "dbus"
	                        // 0x910000 "ave0"
	reg(USB);               // 0x980000
	pad(USB, DMA15);        // 0xe00000 SDRAM Controller
	                        // 0xe01000 L2 Cache Controller
	                        // 0xe02000 L1 Cache Controller
	                        // 0xe04000 "arbr"
	reg(DMA15);             // 0xe05000
	pad(DMA15, HW_END);     // 0xe06000 dram?
	                        // 0xe07000 "dcrc"
	                        // 0xe08000 "AXIP"
} raspi_peripherals;


/// Hardware registers base pointer. See @ref HW.
extern raspi_peripherals *pHW;

/**
 * Accessor to parent structure containing all hardware peripherals.  For
 * example, the UART0 line control register can be accessed as `HW.UART0.LCRH`.
 * To access the register as a single 32-bit unsigned value, use
 * `HW.UART0.LCRH.U`.  To access just the parity enable bit `PEN`, use
 * `HW.UART0.LCRH.B.PEN`.
 *
 * Names follow those listed in the official datasheet, with common prefixes and
 * suffixes (if any) removed.  See @ref registers for a list of available
 * entries.
 */
#define HW (*pHW)


/// Map the hardware registers into the current user-space process.  Return true
/// if successful, false on error.
extern int raspi_map_hw(void);

/// Clock frequency in Hz of the APB (Advanced Peripheral Bus).
#define CORE_CLOCK 250000000
// TODO: Dynamically read from clock manager registers where required.

#ifdef __arm__

/// Stall any following memory access until those before this call have
/// completed.  Always call this after using a peripheral or in between using
/// two different peripherals!  The peripheral bus may mangle your memory
/// accesses otherwise.
#define memory_barrier() asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" :: [zero] "r" (0))

/// Stall execution until all pending memory accesses have completed.
#define synchronization_barrier() asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" :: [zero] "r" (0))

#else
#define memory_barrier() /**/
#define synchronization_barrier() /**/
#endif


/// Map peripheral register offset to physical address space as seen by the ARM
/// CPU.
#define ARM(x) ((x)+0x20000000ul)

/// Map peripheral register offset to bus addresses as seen by the DMA
/// controller and other peripherals.
#define BUS(x) ((x)+0x7e000000ul)


/**
 * @defgroup gpio General-Purpose I/O (GPIO)
 *
 * Helper functions for configuring and accessing GPIO pins.
 *
 * Declared in `raspi/hw.h`.
 * @{
 */

/// Select between GPIO input, GPIO output, or one of the alternate (peripheral)
/// functions.
typedef enum {
	Input = 0,
	Output = 1,
	Alt0 = 4,
	Alt1 = 5,
	Alt2 = 6,
	Alt3 = 7,
	Alt4 = 3,
	Alt5 = 2
} raspi_GPIO_function;


/// Configure GPIO _gpio_ for function _function_.
static inline void gpio_configure(int gpio, raspi_GPIO_function function)
{
	uint32_t tmp = HW.GPIO.FSEL[gpio/10] & ~(7 << ((gpio%10)*3));
	HW.GPIO.FSEL[gpio/10] = tmp | (function&7) << ((gpio%10)*3);
}


/// Set (to logical high) GPIO output _gpio_.
static inline void gpio_set(int gpio)
{
	HW.GPIO.SET[gpio/32] = 1<<(gpio%32);
}


/// Clear (set to logical low) GPIO output _gpio_.
static inline void gpio_clear(int gpio)
{
	HW.GPIO.CLR[gpio/32] = 1<<(gpio%32);
}


/// Return true (but not neccessarily 1) if GPIO input _gpio_ is driven high.
static inline uint32_t gpio_read(int gpio)
{
	return HW.GPIO.LEV[gpio/32] & (1<<(gpio%32));
}

/**
 * @}
 * @defgroup systimer System Timer
 * Utility functions for measuring time and busy-waiting short amounts of time.
 *
 * Declared in `raspi/hw.h`.
 * @{
 */


/// Type for system timer time stamps.
typedef uint32_t st_time_t;


/// Type for system timer time stamp differences.
typedef uint32_t st_delta_t;


/// Return current system timer timestamp.  It measures time independent of
/// clock scaling.
#define ST_NOW ((st_time_t)HW.ST.CLO)


/// System timer frequency in Hz (== timer ticks in 1 s)
#define ST_1s ((st_delta_t)1000000)


/// System timer ticks in 1 ms
#define ST_1ms (ST_1s/1000)


/// System timer ticks in 1 µs
#define ST_1us (ST_1s/1000000)


/// Return true if _after_ is at least _diff_ ticks after _before_.
static inline int st_elapsed(st_time_t before, st_time_t after, st_delta_t diff)
{
	// this is safe across unsigned integer overflows
	return ((after) - (before) >= (diff));
}


/// Busy-wait for the given _delay_.
static inline void st_delay(st_delta_t delay)
{
	st_time_t start = ST_NOW;
	while (!st_elapsed(start, ST_NOW, delay));
}

/**
 * @}
 */

#endif
