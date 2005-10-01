/*
 *  ydpdict
 *  (c) 1998-2003 wojtek kaniewski <wojtekka@irc.pl>
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

#include "config.h"
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_LINUX_SOUNDCARD_H
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <linux/soundcard.h>
#endif

#include "ydpconfig.h"
#include "ydpsound.h"
#include "xmalloc.h"

#define blah(x) { xfree(sample); close(fd); return -x; }

int playsample(int def)
{
	static u_char buf[256];
  
	snprintf(buf, sizeof(buf), "%s/S%.3d/%.6d.WAV", cdpath, def / 1000, def + 1);
	
	if (access(buf, R_OK)) {
		snprintf(buf, sizeof(buf), "%s/s%.3d/%.6d.wav", cdpath, def / 1000, def + 1);

		if (access(buf, R_OK)) {
			snprintf(buf, sizeof(buf), "%s/S%.3d/%.6d.MP3", cdpath, def / 1000, def + 1);

			if (access(buf, R_OK)) {
				snprintf(buf, sizeof(buf), "%s/s%.3d/%.6d.mp3", cdpath, def / 1000, def + 1);

				if (access(buf, R_OK))
					return 0;
			}
		}
	}

	if (player) {
		u_char buf2[512];
    
		snprintf(buf2, sizeof(buf2), "%s %s 2>&1 > /dev/null", player, buf);
		system(buf2);
		return 0;
	}

#ifdef HAVE_LINUX_SOUNDCARD_H
	{
		struct riff_header_type riff_header;
		struct riff_block_type riff_block;
		struct wave_fmt_type wave_fmt;
		struct wave_pcm_type wave_pcm;
		int fd, rd, bits, chans, freq;
		char *sample = NULL;

		/* sprawd¼my czy to w³a¶ciwy plik... */
		if ((fd = open(buf, O_RDONLY)) == -1)
			blah(0);
		if (read(fd, &riff_header, 12) < 12)
			blah(1);
		if (strncmp(riff_header.id, "RIFF", 4))
			blah(2);
		if (strncmp(riff_header.format, "WAVE", 4))
			blah(3);
	  
		for (;;) {
			if (read(fd, &riff_block, 8) < 8)
				blah(4);
			rd = lseek(fd, 0, SEEK_CUR);
			
			if (!strncmp(riff_block.id, "fmt ", 4)) {
				if (read(fd, &wave_fmt, 14) < 14)
					blah(5);
				switch (wave_fmt.wFormatTag) {
					case WAVE_FORMAT_PCM:
						if (read(fd, &wave_pcm, 2) < 2)
							blah(7);
						lseek(fd, riff_block.size - 16, SEEK_CUR);
						break;
					case WAVE_FORMAT_ADPCM:
						blah(6);
						break;
					default:
						blah(6);
				}
				continue;
			}
			
			if (!strncmp(riff_block.id, "data", 4)) {
				 /* wczytaj próbkê */
				sample = xmalloc(riff_block.size);
				rd = read(fd, sample, riff_block.size);
				close(fd);
	      
				/* przygotuj, co trzeba */
				freq = wave_fmt.dwSamplesPerSec;
				bits = wave_pcm.wBitsPerSample;
				chans = wave_fmt.wChannels;

				/* maestro... zaczynamy! */
				if ((fd = open("/dev/dsp", O_WRONLY)) == -1)
					blah(9);
				if (ioctl(fd, SNDCTL_DSP_SPEED, &freq) == -1)
					blah(10);
				if (ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &bits) == -1)
					blah(11);
				if (ioctl(fd, SNDCTL_DSP_CHANNELS, &chans) == -1)
					blah(12);
				if (write(fd, sample, rd) == -1)
					blah(13);
				close(fd);  
	      
				/* posprz±taj */
				xfree(sample);
				break;
			}

			lseek(fd, riff_block.size, SEEK_CUR);
		}
	}
#endif /* HAVE_LINUX_SOUNDCARD_H */
  
	return 1;
}
