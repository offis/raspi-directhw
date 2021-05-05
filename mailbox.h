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
 * @defgroup system Firmware Mailbox Interface
 *
 * These functions allow direct access to the Raspberry Pi's firmware without
 * using the regular Linux device driver. This is useful when running under
 * Xenomai or a similar real-time OS.
 *
 * Declared in `mailbox.h`.
 *
 * @{
 */

#ifndef RASPI_DIRECTHW_MAILBOX_H
#define RASPI_DIRECTHW_MAILBOX_H

#include "hw.h"

typedef enum {
	PROP_REQUEST = 0,
	PROP_RESPONSE_SUCCESS = 0x80000000,
	PROP_RESPONSE_ERROR = 0x80000001
} mbox_property_code_t;

typedef struct {
	uint32_t buffer_size;
	uint32_t code;
} mbox_property_header_t __attribute__((aligned(16)));

typedef struct {
	uint32_t tag;
	uint32_t buffer_size;
	uint32_t response_length:31;
	uint32_t has_response:1;
} mbox_tag_header_t;

// see https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
typedef enum {
	MBOX_TAG_GET_FIRMWARE = 0x00000001,
	// TODO
	MBOX_TAG_GET_CLOCK_STATE = 0x00030001,
	MBOX_TAG_GET_CLOCK_RATE = 0x00030002,
	MBOX_TAG_GET_CLOCK_RATE_MEASURED = 0x00030047,
	MBOX_TAG_SET_CLOCK_STATE = 0x00038001,
	MBOX_TAG_SET_CLOCK_RATE = 0x00038002
} mbox_tag_t;

static inline void mbox_write(raspi_MBOX_CHANNEL_t chan, void *addr) {
	while (HW.MBOX0.STATUS_FULL);
	HW.MBOX1.DATA = chan | (intptr_t)addr;
	memory_barrier();
}

static inline uint32_t mbox_read() {
	memory_barrier();
	while (HW.MBOX0.STATUS_EMPTY);
	return HW.MBOX0.DATA;
}

static inline void mbox_call(raspi_MBOX_CHANNEL_t chan, void *addr) {
	uint32_t wanted = chan | (intptr_t)addr;

	while (HW.MBOX0.STATUS_FULL);
	HW.MBOX1.DATA = wanted;

	do {
		memory_barrier();
		while (HW.MBOX0.STATUS_EMPTY);
	} while (HW.MBOX0.DATA != wanted);
}

#define mbox_property_struct(size) \
	struct { \
		mbox_property_header_t header; \
		mbox_tag_header_t tag; \
		uint32_t data[size]; \
		uint32_t end; \
	}

#define mbox_property_call(tagid, mbox) \
	mbox.header.buffer_size = sizeof(mbox); \
	mbox.header.code = PROP_REQUEST; \
	mbox.tag.tag = tagid; \
	mbox.tag.buffer_size = sizeof(mbox.data); \
	mbox.tag.response_length = sizeof(mbox.data); \
	mbox.tag.has_response = 0; \
	mbox.end = 0; \
	mbox_call(MBOX_CH_PROPVC, &mbox);

#define mbox_property_call_multi(mbox) \
	mbox.header.buffer_size = sizeof(mbox); \
	mbox.header.code = PROP_REQUEST; \
	mbox.end = 0; \
	mbox_call(MBOX_CH_PROPVC, &mbox);

/// clocks settable via mailbox property API
typedef enum {
	MBOX_CLOCK_reserved,
	MBOX_CLOCK_EMMC,
	MBOX_CLOCK_UART,
	MBOX_CLOCK_ARM,
	MBOX_CLOCK_CORE,
	MBOX_CLOCK_V3D,
	MBOX_CLOCK_H264,
	MBOX_CLOCK_ISP,
	MBOX_CLOCK_SDRAM,
	MBOX_CLOCK_PIXEL,
	MBOX_CLOCK_PWM,
	MBOX_CLOCK_HEVC,
	MBOX_CLOCK_EMMC2,
	MBOX_CLOCK_M2MC,
	MBOX_CLOCK_PIXEL_BVB,
} mbox_property_clock_t;


static inline void mbox_set_clock(mbox_property_clock_t clock, uint32_t freq) {
	mbox_property_struct(3) mbox;
	mbox.data[0] = clock;
	mbox.data[1] = 1;
	mbox_property_call(MBOX_TAG_SET_CLOCK_STATE, mbox);

	mbox.data[0] = clock;
	mbox.data[1] = freq;
	mbox.data[2] = 0;
	mbox_property_call(MBOX_TAG_SET_CLOCK_RATE, mbox);
}

static inline uint32_t mbox_get_clock(mbox_property_clock_t clock) {
	mbox_property_struct(2) mbox;
	mbox.data[0] = clock;
	mbox.data[1] = 0;
	mbox_property_call(MBOX_TAG_GET_CLOCK_STATE, mbox);
	if (!mbox.data[1]) return 0;

	mbox.data[0] = clock;
	mbox.data[1] = 0;
	mbox_property_call(MBOX_TAG_GET_CLOCK_RATE, mbox);
	return mbox.data[1];
}

static inline uint32_t mbox_get_clock_measured(mbox_property_clock_t clock) {
	mbox_property_struct(2) mbox;
	mbox.data[0] = clock;
	mbox.data[1] = 0;
	mbox_property_call(MBOX_TAG_GET_CLOCK_RATE_MEASURED, mbox);
	return mbox.data[1];
}

#endif

//@}
