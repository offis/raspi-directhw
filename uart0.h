/**
 * @file
 *
 * This file contains the whole API as static inline functions, since all
 * functions are very short.  There is no accompanying C file.
 *
 * License
 * -------
 *
 * Copyright (c) 2013-2019 OFFIS e.V.
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
 * @defgroup uart Asynchronous Serial Interface (UART0)
 *
 * These functions allow direct access to the Raspberry Pi's full-featured UART
 * (RS-232) controller without using the regular Linux device driver. This is
 * useful when running under Xenomai or a similar real-time OS.
 *
 * Declared in `uart0.h`.
 *
 * @{
 */

#ifndef RASPI_DIRECTHW_UART0_H
#define RASPI_DIRECTHW_UART0_H

#include "hw.h"
#include "mailbox.h"

/// Configure UART hardware for given bit rate.  115200 bit/s is the fastest
/// speed available.
static inline void uart0_init(unsigned int bitrate)
{
	const uint32_t UARTCLK = 3000000;
	mbox_set_clock(MBOX_CLOCK_UART, UARTCLK);

	HW.UART0.CR.B.UARTEN = 0;
	while (HW.UART0.FR.B.BUSY);

	HW.UART0.CR.U = 0;
	HW.UART0.CR.B.TXE = 1;
	HW.UART0.CR.B.RXE = 1;

	HW.UART0.LCRH.U = 0;
	HW.UART0.LCRH.B.FEN = 1;
	HW.UART0.LCRH.B.WLEN = 3;

	HW.UART0.IBRD.B.IBRD = UARTCLK/bitrate/16;
	HW.UART0.FBRD.B.FBRD = UARTCLK*4/bitrate;
	memory_barrier();

	HW.UART0.CR.B.UARTEN = 1;
	gpio_configure(14, Alt0, PullOff);
	gpio_configure(15, Alt0, PullOff);
	memory_barrier();
}


/// Return true if at least _num_ bytes are available in the receive FIFO.  Only
/// guaranteed to work correctly with num == 1. Larger values may return true
/// even if less bytes (but at least one) are available.
static inline int uart0_poll(int num)
{
	if (num <= 0) return 1;
	return !HW.UART0.FR.B.RXFE;
}


/// Read a single byte received via UART. Block if FIFO is currently empty.
static inline uint8_t uart0_read(void)
{
	while (HW.UART0.FR.B.RXFE);
	return HW.UART0.DR.B.DATA;
}


/// Send _data_ via UART.  Block if FIFO is currently full.
static inline void uart0_write(uint8_t data)
{
	while (HW.UART0.FR.B.TXFF);
	HW.UART0.DR.B.DATA = data;
}


/// Block until transmit FIFO is empty.
static inline void uart0_flush()
{
	while (!HW.UART0.FR.B.TXFE || HW.UART0.FR.B.BUSY);
}


#endif

//@}
