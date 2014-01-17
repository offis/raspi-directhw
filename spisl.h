/**
 * @file
 *
 * This file contains the whole API as static inline functions, since all
 * functions are very short.  There is no accompanying C file.
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
 * @defgroup spisl SPI Slave (via IIS/PCM)
 *
 * These functions allow the Raspberry Pi to act as SPI slave, even though it
 * has some serious limitations.  The natve SPI slave component is not connected
 * to the board, so this is the only way to get some kind of SPI slave mode of
 * operation. Note that you *must* unload the PCM kernel module, or these
 * functions will not work correctly.
 *
 * It (ab)uses the PCM interface for this, but there is no way to synchronize to
 * byte boundaries.  You must synchronize to the master clock in some way.
 * spisl_synchronize() does so by expecting a continuous byte stream and
 * glitching the clock until the bytes read correctly.
 *
 * Declared in `raspi/spisl.h`.
 *
 * Wiring
 * ------
 *
 * On the Raspberry Pi, this uses the unpopulated connector P5, which is only
 * available on Rev. 2 boards.  It looks like this:
 *
 *     ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
 *     ┃ ┏━━━━━━━━━┓ P1                      ┃
 *     ┃ ┗━━━━━━━━━┛ (GPIO pin header)       ┃
 *     ┃ P5 ┏━━━┓                       ┏━━━━━┓
 *     ┃    ┗━━━┛       ┏━━━━━┓         ┃ USB ┃
 *     ┃ (unpopulated)  ┃ CPU ┃         ┗━━━━━┛
 *     ┃                ┗━━━━━┛         ┏━━━━━┓
 *     ┃                                ┃ LAN ┃
 *     ┃      Raspberry Pi (top view)   ┗━━━━━┛
 *     ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 *
 *     ┏━━━━━━━━━━━━━━━━━━━━━━━┓
 *     ┃ +5V   SCLK  MOSI  GND ┃  P5 pin mapping
 *     ┃ +3.3V  CS   MISO  GND ┃  (top view)
 *     ┗━━━━━━━━━━━━━━━━━━━━━━━┛
 *
 * Note that CS (Frame Sync) is not actually used.  The Pi *must* be the only
 * slave on the bus, and you should wire CS to GND.
 *
 * @{
 */

#ifndef RASPI_SPISL_H
#define RASPI_SPISL_H

#include "raspi/hw.h"
#include <stdint.h>


/// Configure PCM hardware to act as SPI slave.
static inline void spisl_init()
{
	int i;

	struct raspi_PCM_CS_reg init_cs = {
		.TXCLR = 1,
		.RXCLR = 1,
		.TXERR = 1,
		.RXERR = 1,
		.TXON = 1,
		.RXON = 1,
		.SYNC = 1,
		.STBY = 1,
		.EN = 1,
	};

	const struct raspi_PCM_MODE_reg mode_master = {
		.FLEN = 7,
		.FSLEN = 7,
		.CLKM = 0,
		.CLKI = 0,
		.FSM = 0,
		.FSI = 0,
		.CLK_DIS = 1,
	};

	const struct raspi_PCM_MODE_reg mode_slave = {
		.FLEN = 7,
		.FSLEN = 7,
		.CLKM = 1,
		.CLKI = 0,
		.FSM = 1,
		.FSI = 1,
		.CLK_DIS = 1,
	};

	const struct raspi_CM_CTL_reg pcm_cm_ctl = {
		.PASSWD = CM_PASSWD,
		.SRC = CM_OSC,
		.MASH = 1,
		.ENAB = 1,
	};

	const struct raspi_CM_DIV_reg pcm_cm_div = {
		.PASSWD = CM_PASSWD,
		.DIVI = 250,
	};

	// disable interface for reconfiguration
	for (i = 28; i <= 31; i++) gpio_configure(i, Input);
	memory_barrier();

	// empty FIFO
	while (HW.PCM.CS.B.RXD) (void)HW.PCM.FIFO;

	// disable module to reset all parameters
	HW.PCM.CS.B.EN = 0;
	st_delay(ST_1ms);

	// set desired transmission parameters
	HW.PCM.RXC.U = 0;
	HW.PCM.TXC.U = 0;
	HW.PCM.INTEN.U = 0;
	HW.PCM.INTSTC.U = 15;
	HW.PCM.GRAY.U = 0;

	HW.PCM.RXC.B.CH1WEX = 0;
	HW.PCM.RXC.B.CH1POS = 0;
	HW.PCM.RXC.B.CH1WID = 0;
	HW.PCM.RXC.B.CH1EN = 1;

	HW.PCM.TXC.B.CH1WEX = 0;
	HW.PCM.TXC.B.CH1POS = 0;
	HW.PCM.TXC.B.CH1WID = 0;
	HW.PCM.TXC.B.CH1EN = 1;

	// execute reset sequence
	HW.CM[CM_PCM].DIV.B = pcm_cm_div;
	HW.CM[CM_PCM].CTL.B = pcm_cm_ctl;
	HW.PCM.MODE.B = mode_master;
	HW.PCM.MODE.B.CLK_DIS = 0;
	HW.PCM.CS.B = init_cs;

	while (!HW.PCM.CS.B.SYNC);
	HW.PCM.CS.B.SYNC = 0;
	while (HW.PCM.CS.B.SYNC);

	// configure slave mode and attach external interface
	HW.PCM.MODE.B.CLK_DIS = 1;
	HW.PCM.MODE.B = mode_slave;

	memory_barrier();
	gpio_configure(28, Alt2);
	gpio_configure(29, Alt2);
	gpio_configure(30, Alt2);
	gpio_configure(31, Alt2);
	memory_barrier();

	HW.PCM.MODE.B.CLK_DIS = 0;

	memory_barrier();
}


/// Return true if at least _num_ bytes are available in the receive FIFO.  Only
/// guaranteed to work correctly with num == 1. Larger values may return true
/// even if less bytes (but at least one) are available.
static inline int spisl_poll(int num)
{
	if (num <= 0) return 1;
	return HW.PCM.CS.B.RXD;
}


/// Read a single byte received via SPI. Block if FIFO is currently empty.
static inline uint8_t spisl_read(void)
{
	while (!HW.PCM.CS.B.RXD);
	return HW.PCM.FIFO;
}


/// Send _data_ via SPI.  Block if FIFO is currently full.  Note that an SPI
/// slave only sends while receiving, so _data_ is only sent while the master
/// transmits a new byte, which may be much later (or never).
static inline void spisl_write(uint8_t data)
{
	uint8_t dummy;
	while (!HW.PCM.CS.B.TXD);
	HW.PCM.FIFO = data;
	while (HW.PCM.CS.B.RXD) dummy = HW.PCM.FIFO;
	(void)dummy;
}


/// Block until transmit FIFO is empty.
static inline void spisl_flush()
{
	while (!HW.PCM.CS.B.TXE);
}



/**
 *  Synchronize to SPI master.  Due to the way the Raspberry Pi PCM interface
 *  works, bytes are usually not aligned correctly.  To establish correct
 *  transmission, the master should transmit a stream of marker bytes (0x81)
 *  until it reads the marker byte back.  This function will adjust reception
 *  parameters until the marker bytes come through correctly, then send a marker
 *  byte as acknowledgement.  Finally, the master sends (marker ^ 0xff) to
 *  finish synchronization.
 *
 *  You can use this function as template in case you require some other way of
 *  synchronization, e.g. when you don't have the option to modify the master's
 *  program code.  In any case, there is no way to reliably synchronize master
 *  and slave without reading (and discarding) several hundreds of bytes.  The
 *  Frame Sync input is not reliable enough for this purpose, unfortunately.
 */
static inline void spisl_synchronize(void)
{
	uint8_t marker = 0x81;
	uint8_t incoming;
	int cnt = 0;

	incoming = spisl_read();
	while (cnt++ < 10) {
		if (incoming != marker) {
			HW.PCM.MODE.B.CLK_DIS = 1;
			st_delay(ST_1us);
			cnt = 0;
			HW.PCM.MODE.B.CLK_DIS = 0;
		}
		incoming = spisl_read();
	}

	spisl_write(marker);

	marker ^= 0xff;

	while (incoming != marker) incoming = spisl_read();
}

#endif

///@}
