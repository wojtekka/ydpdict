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
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydpcore.h"
#include "ydpconvert.h"
#include "xmalloc.h"

#define max(a,b) ((a > b) ? a : b)

unsigned long fix32(unsigned long x)
{
#ifndef WORDS_BIGENDIAN
	return x;
#else
	return (unsigned long)
		(((x & (unsigned long) 0x000000ffU) << 24) |
                 ((x & (unsigned long) 0x0000ff00U) << 8) |
                 ((x & (unsigned long) 0x00ff0000U) >> 8) |
                 ((x & (unsigned long) 0xff000000U) >> 24));
#endif		
}

unsigned short fix16(unsigned short x)
{
#ifndef WORDS_BIGENDIAN
	return x;
#else
	return (unsigned short)
		(((x & (unsigned short) 0x00ffU) << 8) |
                 ((x & (unsigned short) 0xff00U) >> 8));
#endif
}

/* otwiera s³ownik */
int opendict(const u_char *path, const u_char *index, const u_char *def)
{
	int current = 0, bp = 0;
	unsigned long pos;
	u_char buf[256], ch, *p;
	size_t pathsize = max(strlen(index), strlen(def)) + strlen(path) + 2;
	
	ydperror = YDP_NONE;

	/* otwórz plik indeksowy i plik z definicjami */
	p = xmalloc(pathsize);
	
	snprintf(p, pathsize, "%s/%s", path, index);
	
	if ((fi = fopen(p, "r")) == NULL) {
		char *q;

		for (q = strrchr(p, '/'); *q; q++)
			*q = toupper(*q);

		if ((fi = fopen(p, "r")) == NULL) {
			xfree(p);
			ydperror = YDP_CANTOPENIDX;
			return 0;
		}
	}
	
	snprintf(p, pathsize, "%s/%s", path, def);
	
	if ((fd = fopen(p, "r")) == NULL) {
		char *q;

		for (q = strrchr(p, '/'); *q; q++) 
			*q = toupper(*q);

		if ((fd = fopen(p, "r")) == NULL) {
			fclose(fi);
			xfree(p);
			ydperror = YDP_CANTOPENDEF;
			return 0;
		}
	}

	xfree(p);

	/* wczytaj ilo¶æ s³ów */
	fseek(fi, 0x08, SEEK_SET);
	wordcount = 0;
	fread(&wordcount, 1, 2, fi);
	wordcount = fix16(wordcount);

	/* zarezerwuj odpowiedni± ilo¶æ pamiêci */
	indexes = xmalloc(wordcount * sizeof(unsigned long));
	words = xmalloc((wordcount + 1) * sizeof(char*));
	words[wordcount] = 0;
    
	/* wczytaj offset tablicy indeksów */
	fseek(fi, 0x10, SEEK_SET);
	pos = 0;
	fread(&pos, 1, 0x04, fi);
	pos = fix32(pos);
	fseek(fi, pos, SEEK_SET);

	/* wczytaj tablicê indeksów */
	do {
		fseek(fi, 0x04, SEEK_CUR);
		indexes[current] = 0;
		fread(&indexes[current], 1, sizeof(unsigned long), fi);
		indexes[current] = fix32(indexes[current]);

		bp = 0;
		do {
			fread(&ch, 1, 1, fi);
			buf[(bp < 255) ? bp++ : bp] = ch;
		} while(ch);

		words[current] = xstrdup(buf);

		convert_cp1250(words[current], 0);

	} while (++current < wordcount);

	return 1;
}

/* wczytuje definicjê */
u_char *readdef(int i)
{
        unsigned long dsize;
        u_char *def;

        /* za³aduj definicjê do bufora */
        fseek(fd, indexes[i], SEEK_SET);
        dsize = 0;
	fread(&dsize, 1, sizeof(unsigned long), fd);
        dsize = fix32(dsize);

        def = xmalloc(dsize + 1);

        if (fread(def, 1, dsize, fd) != dsize) {
                xfree(def);
                ydperror = YDP_INVALIDFILE;
                return 0;
        }

        def[dsize] = 0;
        convert_cp1250(def, 0);

        ydperror = YDP_NONE;
        return def;
}

/* pozbywa siê liter, które zaszkodzi³yby strncasecmp() */
static const u_char *lower_pl(const u_char *word)
{
	static u_char buf[128];
	int i = 0;

#define _c(x) { u_char y = (u_char) x; buf[i++] = y; break; }

	while (word[i]) {
		switch (word[i]) {
			case (u_char) '¡': _c('±');
			case (u_char) 'Æ': _c('æ');
			case (u_char) 'Ê': _c('ê');
			case (u_char) '£': _c('³');
			case (u_char) 'Ñ': _c('ñ');
			case (u_char) 'Ó': _c('ó');
			case (u_char) '¦': _c('¶');
			case (u_char) '¬': _c('¼');
			case (u_char) '¯': _c('¿');
			default: _c(word[i]);
		}
	}

#undef _c

	buf[i] = 0;

	return buf;
}

/* znajduje indeks s³owa zaczynaj±cego siê od podanego tekstu */
int findword(const u_char *word)
{
	int x = 0;
	
	for (; x < wordcount; x++)
		if (!strncasecmp(words[x], lower_pl(word), strlen(word)))
			return x;
	
	return -1;
}

/* zamyka pliki i zwalnia pamiêæ */
void closedict()
{
	int x = 0;
  
	xfree(indexes);
	
	if (words) {
		while (words[x]) {
			xfree(words[x]);
			x++;
		}
		
		xfree(words);
	}
	
	fclose(fd);
	fclose(fi);
}
