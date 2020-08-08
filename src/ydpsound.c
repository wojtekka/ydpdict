/*
 *  ydpdict
 *  (c) Copyright 1998-2009 Wojtek Kaniewski <wojtekka@toxygen.net>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydpconfig.h"
#include "ydpsound.h"
#include "xmalloc.h"
#include "adpcm.h"

#ifdef HAVE_LINUX_SOUNDCARD_H
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#endif

#ifdef HAVE_LIBAO
#include <ao/ao.h>
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif

/**
 * \brief Convert 32-bit value from little-endian to machine-endian
 *
 * \param value little-endian value
 * 
 * \return machine-endian value
 */
static inline void fix32(uint32_t *value)
{
#ifdef WORDS_BIGENDIAN
	*value = (uint32_t) (((*value & (uint32_t) 0x000000ffU) << 24) |
		((*value & (uint32_t) 0x0000ff00U) << 8) |
		((*value & (uint32_t) 0x00ff0000U) >> 8) |
		((*value & (uint32_t) 0xff000000U) >> 24));
#endif		
}

/**
 * \brief Convert 16-bit value from little-endian to machine-endian
 *
 * \param value little-endian value
 * 
 * \return machine-endian value
 */
static inline void fix16(uint16_t *value)
{
#ifdef WORDS_BIGENDIAN
	*value = (uint16_t) (((*value & (uint16_t) 0x00ffU) << 8) |
		((*value & (uint16_t) 0xff00U) >> 8));
#endif
}

#ifdef HAVE_LIBAO

/**
 * \brief Play sample using libao
 *
 * \param sample Sample data
 * \param size Sample size in bytes
 * \param freq Sampling frequency
 * \param bits Bits per sample
 * \param chans Channels
 *
 * \return 0 on success, -1 on error
 */
static int play_libao(char *sample, int size, int freq, int bits, int chans)
{
	ao_device *device;
	ao_sample_format format;
	int driver;

	if (!config_audio)
		driver = ao_default_driver_id();
	else
		driver = ao_driver_id(config_audio);

	if (driver == -1)
		return -1;

	memset(&format, 0, sizeof(format));
	format.bits = bits;
	format.channels = chans;
	format.rate = freq;
	format.byte_format = AO_FMT_NATIVE;

	if (!(device = ao_open_live(driver, &format, NULL)))
		return -1;
	
	ao_play(device, sample, size);

	ao_close(device);

	return 0;
}

#else /* HAVE_LIBAO */

#ifdef HAVE_LINUX_SOUNDCARD_H

/**
 * \brief Play sample using Linux OSS interface
 *
 * \param sample Sample data
 * \param size Sample size in bytes
 * \param freq Sampling frequency
 * \param bits Bits per sample
 * \param chans Channels
 *
 * \return 0 on success, -1 on error
 */
static int play_linux(char *sample, int size, int freq, int bits, int chans)
{
	int fd;
	
	if (!config_audio)
		return -1;

	if ((fd = open(config_audio, O_WRONLY)) == -1)
		return -1;

	if (ioctl(fd, SNDCTL_DSP_SPEED, &freq) == -1) {
		close(fd);
		return -1;
	}

	if (ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &bits) == -1) {
		close(fd);
		return -1;
	}

	if (ioctl(fd, SNDCTL_DSP_CHANNELS, &chans) == -1) {
		close(fd);
		return -1;
	}

	while (size > 0) {
		int res;

		if ((res = write(fd, sample, size)) == -1) {
			close(fd);
			return -1;
		}

		size -= res;
		sample += res;
	}

	close(fd);

	return 0;
}

#endif /* HAVE_LINUX_SOUNDCARD_H */

#endif /* HAVE_LIBAO */
	
/**
 * \brief Plays sample for pronunciation
 * 
 * \param dict Dictionary type
 * \param def Word index
 *
 * \return 0 on success, -1 on invalid file format, -2 on invalid file codec, -3 on playback error
 */
int play_sample(int dict, int def)
{
	char *exts[] = { "WAV", "wav", "MP3", "mp3", "OGG", "ogg", NULL };
	struct riff_header_type riff_header;
	struct riff_block_type riff_block;
	struct wave_fmt_type wave_fmt;
	struct wave_adpcm_type wave_adpcm;
	int i, fd = -1, rd, result;
	char path[4096], *sample = NULL;
	int bytesPerBlock;
	short *coefs = NULL;

	for (i = 0; exts[i]; i++) {
		if (dict == 0) {
			snprintf(path, sizeof(path), "%s/en/S%.3d/%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
			if (!access(path, R_OK))
				break;

			snprintf(path, sizeof(path), "%s/en/s%.3d/%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
			if (!access(path, R_OK))
				break;
		}

		if (dict == 2) {
			snprintf(path, sizeof(path), "%s/de/sg%.3d/sg%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
			if (!access(path, R_OK))
				break;

			snprintf(path, sizeof(path), "%s/de/SG%.3d/SG%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
			if (!access(path, R_OK))
				break;
		}

		snprintf(path, sizeof(path), "%s/S%.3d/%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
		if (!access(path, R_OK))
			break;
		
		snprintf(path, sizeof(path), "%s/s%.3d/%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);

		if (!access(path, R_OK))
			break;
	
		snprintf(path, sizeof(path), "%s/sg%.3d/sg%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
		if (!access(path, R_OK))
			break;

		snprintf(path, sizeof(path), "%s/SG%.3d/SG%.6d.%s", config_cdpath, (def + 1) / 1000, def + 1, exts[i]);
	
		if (!access(path, R_OK))
			break;
	}

	if (!exts[i])
		return 0;

	if (config_player) {
		char command[4096];
    
		snprintf(command, sizeof(command), "%s %s > /dev/null 2> /dev/null", config_player, path);
		return system(command);
	}

	if ((fd = open(path, O_RDONLY)) == -1) {
		result = 0;
		goto failure;
	}
	
	if (read(fd, &riff_header, sizeof(riff_header)) < sizeof(riff_header)) {
		result = -1;
		goto failure;
	}

	fix32(&riff_header.size);

	if (strncmp(riff_header.id, "RIFF", sizeof(riff_header.id))) {
		result = -1;
		goto failure;
	}

	if (strncmp(riff_header.format, "WAVE", sizeof(riff_header.format))) {
		result = -1;
		goto failure;
	}
  
	for (;;) {
		if (read(fd, &riff_block, sizeof(riff_block)) < sizeof(riff_block)) {
			result = -1;
			goto failure;
		}

		fix32(&riff_block.size);

		rd = lseek(fd, 0, SEEK_CUR);
			
		if (!strncmp(riff_block.id, "fmt ", sizeof(riff_block.id))) {
			if (read(fd, &wave_fmt, sizeof(wave_fmt)) < sizeof(wave_fmt)) {
				result = -1;
				goto failure;
			}

			fix16(&wave_fmt.wFormatTag);
			fix16(&wave_fmt.wChannels);
			fix32(&wave_fmt.dwSamplesPerSec);
			fix32(&wave_fmt.dwAvgBytesPerSec);
			fix16(&wave_fmt.wBlockAlign);
			fix16(&wave_fmt.wBitsPerSample);

			switch (wave_fmt.wFormatTag) {
				case WAVE_FORMAT_PCM:
					lseek(fd, riff_block.size - sizeof(wave_fmt), SEEK_CUR);
					break;

				case WAVE_FORMAT_ADPCM:
					if (riff_block.size < sizeof(wave_fmt) + sizeof(wave_adpcm)) {
						result = -1;
						goto failure;
					}

					if (read(fd, &wave_adpcm, sizeof(wave_adpcm)) < sizeof(wave_adpcm)) {
						result = -1;
						goto failure;
					}

					fix16(&wave_adpcm.wExtSize);
					fix16(&wave_adpcm.wSamplesPerBlock);
					fix16(&wave_adpcm.wCoefs);

					bytesPerBlock = 7 * wave_fmt.wChannels;
					
					if (wave_adpcm.wSamplesPerBlock > 2)
						bytesPerBlock += ((wave_adpcm.wSamplesPerBlock - 2) * wave_fmt.wChannels + 1) / 2;
					
					if (riff_block.size < sizeof(wave_fmt) + sizeof(wave_adpcm) + wave_adpcm.wCoefs * 4) {
						result = -1;
						goto failure;
					}
					
					coefs = xmalloc(wave_adpcm.wCoefs * 2 * sizeof(short));

					if (read(fd, coefs, wave_adpcm.wCoefs * 2 * sizeof(short)) != wave_adpcm.wCoefs * 2 * sizeof(short)) {
						result = -1;
						goto failure;
					}

					for (i = 0; i < wave_adpcm.wCoefs; i++)
						fix16((unsigned short*) &coefs[i]);

					lseek(fd, riff_block.size - sizeof(wave_fmt) - sizeof(wave_adpcm) - 4 * wave_adpcm.wCoefs, SEEK_CUR);

					break;

				default:
					result = -2;
					goto failure;
			}
			continue;
		}
		
		if (!strncmp(riff_block.id, "data", 4)) {
			int bits = wave_fmt.wBitsPerSample;

			switch (wave_fmt.wFormatTag) {
				case WAVE_FORMAT_PCM:
				{
					sample = xmalloc(riff_block.size);
					rd = read(fd, sample, riff_block.size);

					break;
				}

				case WAVE_FORMAT_ADPCM:
				{
					int res, count, bytes;
					char *packet;
					short *samples;
					
					packet = xmalloc(wave_fmt.wBlockAlign);
					samples = xmalloc(wave_adpcm.wSamplesPerBlock * sizeof(short));
					bits = 16;
					bytes = 0;
					sample = NULL;
					rd = 0;

					while (bytes < riff_block.size) {
						res = read(fd, packet, wave_fmt.wBlockAlign);
						if (res < 1) {
							result = -2;
							goto failure;
						}

						bytes += res;
						count = wave_adpcm.wSamplesPerBlock;

						if (res < wave_fmt.wBlockAlign) {
							if (!(count = AdpcmSamplesIn(0, wave_fmt.wChannels, res, 0))) {
								result = -2;
								goto failure;
							}
						}

						sample = xrealloc(sample, rd + count * sizeof(short));

						AdpcmBlockExpandI(wave_fmt.wChannels, wave_adpcm.wCoefs, coefs, (unsigned char*) packet, (short*) (sample + rd), count);

						rd += count * sizeof(short);
					}

					xfree(packet);
					xfree(samples);

					break;
				}
			}
      
			close(fd);

#ifdef HAVE_LIBAO
			if (play_libao(sample, rd, wave_fmt.dwSamplesPerSec, bits, wave_fmt.wChannels) == -1) {
				result = -3;
				goto failure;
			}
#else
#ifdef HAVE_LINUX_SOUNDCARD_H
			if (play_linux(sample, rd, wave_fmt.dwSamplesPerSec, bits, wave_fmt.wChannels) == -1) {
				result = -3;
				goto failure;
			}
#endif /* HAVE_LINUX_SOUNDCARD_H */
#endif /* HAVE_LIBAO */
      
			xfree(sample);

			break;
		}

		lseek(fd, riff_block.size, SEEK_CUR);
	}

	return 0;
		
failure:
	xfree(sample);
	xfree(coefs);
	
	if (fd != -1)
		close(fd);

	return result;
}
