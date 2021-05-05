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
 * @defgroup uart Mini Asynchronous Serial Interface (UART1)
 *
 * These functions allow direct access to the Raspberry Pi's Mini UART (RS-232)
 * controller without using the regular Linux device driver.  This is useful
 * when running under Xenomai or a similar real-time OS.
 *
 * Declared in `uart1.h`.
 *
 * @{
 */

#ifndef RASPI_DIRECTHW_UART1_H
#define RASPI_DIRECTHW_UART1_H

#include "hw.h"
#include "mailbox.h"

/// Configure UART hardware for given bit rate.  115200 bit/s is the fastest
/// speed available.
static inline void uart1_init(unsigned int bitrate)
{
	const uint32_t UARTCLK = 250000000;

	gpio_configure(14, Alt5, PullOff);
	gpio_configure(15, Alt5, PullOff);
	memory_barrier();

	HW.AUX.IRQ.B.MINI_UART = 0; // disable mini-UART interrupts
	HW.AUX.ENB.B.MINI_UART = 1; // enable mini-UART
	memory_barrier();

	HW.UART1.CNTL.U = 0; // ignore CTS, manual RTS, receive/transmit disable
	while (!(HW.UART1.STAT.B.RX_IDLE)); // wait until receiver is idle

	HW.UART1.IER.U = 0; // disable interrupts
	HW.UART1.LCR.U = 3; // 8-bit mode

	HW.UART1.BAUD = UARTCLK/(8*bitrate)-1; // set bit rate register

	HW.UART1.IIR.U = 6; // clear FIFOs
	memory_barrier();

	HW.UART1.CNTL.B.TX_ENABLE = 1; // receive/transmit enable
	HW.UART1.CNTL.B.RX_ENABLE = 1; // receive/transmit enable
	memory_barrier();

}


/// Return true if at least _num_ bytes are available in the receive FIFO.  Only
/// guaranteed to work correctly with num == 1. Larger values may return true
/// even if less bytes (but at least one) are available.
static inline int uart1_poll(int num)
{
	if (num <= 0) return 1;
	return !!HW.UART1.STAT.B.RX_READY;
}


/// Read a single byte received via UART. Block if FIFO is currently empty.
static inline uint8_t uart1_read(void)
{
	while (!HW.UART1.STAT.B.RX_READY);
	return HW.UART1.IO.B.DATA;
}


/// Send _data_ via UART.  Block if FIFO is currently full.
static inline void uart1_write(uint8_t data)
{
	while (!HW.UART1.STAT.B.TX_READY);
	HW.UART1.IO.B.DATA = data;
}


/// Block until transmit FIFO is empty.
static inline void uart1_flush()
{
	while (!HW.UART1.STAT.B.TX_DONE);
}


#endif

//@}
