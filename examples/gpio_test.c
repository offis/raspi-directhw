/**
 * @example gpio_test.c
 *
 * Basic GPIO test. Blinks LED D5 (SD-Card activity LED).

 * The blocks surrounded by #`ifdef __XENO__` are used when compiling this
 * example using [Xenomai](http://www.xenomai.org). Use these snippets in your
 * program to get a single source file that can be compiled for both, regular
 * Linux or Xenomai.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "raspi/hw.h"
#include "raspi/hw.c"

#ifdef __XENO__
#include <sys/mman.h>
#include <rtdk.h>
#define printf rt_printf
#endif

int main(int argc, char *argv[])
{
#ifdef __XENO__
	struct sched_param param = { 99 };
	mlockall(MCL_CURRENT | MCL_FUTURE);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
#endif

	if (!raspi_map_hw()) {
		perror("Could not map hardware registers");
		exit(1);
	}

#ifdef __XENO__
	pthread_set_mode_np(0, PTHREAD_WARNSW|PTHREAD_PRIMARY);
#endif

	gpio_configure(16, Output);

	for (;;) {
		gpio_set(16);
		sleep(1);
		gpio_clear(16);
		sleep(1);
	}

#ifdef __XENO__
	pthread_set_mode_np(PTHREAD_WARNSW, 0);
#endif

	return 1;
}
