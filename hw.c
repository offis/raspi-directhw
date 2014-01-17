/**
 * @file
 *
 * Definitions for the hardware register structure HW and initialization
 * function raspi_map_hw().  Anything else is contained in raspi/hw.h.
 *
 * Due to its small size, you may want to #`include` this file in exactly one of
 * your source files instead of compiling and linking it separately.
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

#include "raspi/hw.h"

raspi_peripherals *pHW = (raspi_peripherals *)ARM(0);

#if defined(linux) && !defined(__KERNEL__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int raspi_map_hw(void)
{
	int fd = open("/dev/mem", O_RDWR|O_SYNC);

	if (fd < 0) return 0;

	void *addr = mmap(NULL, sizeof(raspi_peripherals), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED|MAP_POPULATE, fd, ARM(0));

	close(fd);

	if (addr == MAP_FAILED)  return 0;

	pHW = (raspi_peripherals *)addr;

	return 1;
}
#endif
