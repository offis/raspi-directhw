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
 * @defgroup spi SPI Master (SPI0)
 *
 * These functions allow direct access to the Raspberry Pi's SPI0 master
 * peripheral without using the regular Linux device driver.  This is useful
 * when running under Xenomai or a similar real-time OS.  Note that you *must*
 * unload the SPI kernel module, or these functions will not work correctly.
 *
 * Declared in `raspi/spi.h`.
 *
 * @{
 */

#ifndef RASPI_SPI_H
#define RASPI_SPI_H

#include "raspi/hw.h"
#include <stdint.h>

/// Size of both, the read and the write FIFO.
#define raspi_SPI_FIFOSIZE 16


/// Configure SPI hardware for _speed_ bit/s.
static inline void spi_init(uint32_t speed)
{
	int i;
	uint32_t div = CORE_CLOCK / speed;

	if (div >= 65536) div = 0;
	if (div < 2) div = 2;
	HW.SPI0.CLK.B.CDIV = div;
	HW.SPI0.CS.B.CPHA = 1;

	memory_barrier();
	for (i = 7; i <= 11; i++) gpio_configure(i, Alt0);
	memory_barrier();
}


/// Start SPI transfer to _destination_ (0 or 1, corresponding to
/// the available chip select lines). The receive FIFO will be cleared.
static inline void spi_start(int destination)
{
	HW.SPI0.CS.B.CS = destination;
	HW.SPI0.CS.B.CLEAR = 2;
	HW.SPI0.CS.B.TA = 1;
}


/// Stop SPI transfer. Block until all pending data is transmitted.
static inline void spi_stop(void)
{
	while (!HW.SPI0.CS.B.DONE);
	HW.SPI0.CS.B.TA = 0;
}


/// Return true if at least _num_ bytes are available in the receive FIFO.
/// Only guaranteed to work correctly with num == 1. Larger values may
/// return true even if less bytes (but at least one) are available.
static inline int spi_poll(int num)
{
	if (num <= 0) return 1;
	if (num < raspi_SPI_FIFOSIZE*3/4) return HW.SPI0.CS.B.RXD;
	if (num < raspi_SPI_FIFOSIZE) return HW.SPI0.CS.B.RXR;
	return HW.SPI0.CS.B.RXF;
}


/// Read a single byte received via SPI. Block if FIFO is
/// currently empty. Note that SPI only receives while sending,
/// so each `spi_read()` must be paired with an `spi_write()`.
static inline uint8_t spi_read(void)
{
	while (!HW.SPI0.CS.B.RXD);
	return HW.SPI0.FIFO;
}


/// Send _data_ via SPI. Block if FIFO is currently full.
/// `spi_start()` must be called before.
static inline void spi_write(uint8_t data)
{
	while (!HW.SPI0.CS.B.TXD);
	HW.SPI0.FIFO = data;
}


/// Block until transmit FIFO is empty.
static inline void spi_flush(void)
{
	while (HW.SPI0.CS.B.TA && !HW.SPI0.CS.B.DONE);
}

#endif

///@}
