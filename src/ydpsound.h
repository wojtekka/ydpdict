/*
 *  ydpdict
 *  (C) Copyright 1998-2004 Wojtek Kaniewski <wojtekka@toxygen.net>
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

struct riff_header_type {
	char id[4];
	unsigned long size;
	char format[4];
};

struct riff_block_type {
	char id[4];
	unsigned long size;
};

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_ADPCM 0x0002

struct wave_fmt_type {
	unsigned short int wFormatTag;
	unsigned short int wChannels;
	unsigned long dwSamplesPerSec;
	unsigned long dwAvgBytesPerSec;
	unsigned short int wBlockAlign;
};

struct wave_pcm_type {
	unsigned short int wBitsPerSample;
};

int playsample(int def);

#endif /* __YDPSOUND_H */
