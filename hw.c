/**
 * @file
 *
 * Definitions for the hardware register structure HW and initialization
 * function raspi_map_hw().  Anything else is contained in hw.h.
 *
 * Due to its small size, you may want to #`include` this file in exactly one of
 * your source files instead of compiling and linking it separately.
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
 */

#if defined(linux) && !defined(__KERNEL__)

#include "hw.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>

raspi_peripherals *pHW = (raspi_peripherals *)0;

int raspi_map_hw(void)
{
	if (pHW) return 1;

	// Fallback value for original Pi, later ones should always have /proc
	uint32_t arm_io_base = 0x20000000ul;

	int fd = open("/proc/device-tree/soc/ranges", O_RDONLY);
	if (fd != -1) {
		uint8_t ranges[8];
		if (read(fd, &ranges, sizeof(ranges)) == sizeof(ranges)) {
			arm_io_base = (
				(ranges[4]<<24) |
				(ranges[5]<<16) |
				(ranges[6]<<8)  |
				(ranges[7]));
#ifndef NDEBUG
			uint32_t bus_io_base = (
				(ranges[0]<<24) |
				(ranges[1]<<16) |
				(ranges[2]<<8)  |
				(ranges[3]));
			assert(bus_io_base == BUS(0));
#endif
		}
		close(fd);
	}

	fd = open("/dev/mem", O_RDWR|O_SYNC);

	if (fd < 0) return 0;

	void *addr = mmap(NULL, sizeof(raspi_peripherals), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED|MAP_POPULATE, fd, arm_io_base);

	close(fd);

	if (addr == MAP_FAILED)  return 0;

	pHW = (raspi_peripherals *)addr;

	return 1;
}

#endif
