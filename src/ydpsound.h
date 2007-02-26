/*
 *  ydpdict
 *  (C) Copyright 1998-2007 Wojtek Kaniewski <wojtekka@toxygen.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __YDPSOUND_H
#define __YDPSOUND_H

#include <inttypes.h>

struct riff_header_type {
	char id[4];
	uint32_t size;
	char format[4];
} __attribute__ ((packed));

struct riff_block_type {
	char id[4];
	uint32_t size;
} __attribute__ ((packed));

enum wave_fmt_type_type {
	WAVE_FORMAT_PCM = 0x0001,
	WAVE_FORMAT_ADPCM = 0x0002
};

struct wave_fmt_type {
	uint16_t wFormatTag;
	uint16_t wChannels;
	uint32_t dwSamplesPerSec;
	uint32_t dwAvgBytesPerSec;
	uint16_t wBlockAlign;
	uint16_t wBitsPerSample;
} __attribute__ ((packed));

struct wave_adpcm_type {
	uint16_t wExtSize;
	uint16_t wSamplesPerBlock;
	uint16_t wCoefs;
} __attribute__ ((packed));

int play_sample(int def);

#endif /* __YDPSOUND_H */
