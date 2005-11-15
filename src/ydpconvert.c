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

#include <sys/types.h>
#include <string.h>

#include "ydpconvert.h"
#include "xmalloc.h"

/* tablice konwersji */
u_char table_cp_iso[] = TABLE_CP_ISO;
u_char table_iso_plain[] = TABLE_ISO_PLAIN;
u_char *table_unicode[] = TABLE_UNICODE;
u_char *table_phonetic_iso[] = TABLE_PHONETIC_ISO;

u_char *char_table_conv(u_char *inp, u_char *table)
{
	u_char *prev = inp;

	for (; *inp; inp++)
		if (*inp > 127)
			*inp = table[*inp - 128];
  
	return prev;
}
       
u_char *string_table_conv(u_char *inp, u_char **table)
{
	static u_char buf[1024], letter[2] = " \0";
  
	memset(buf, 0, sizeof(buf));
	
	for (; *inp; inp++) {
		if (*inp > 127) {
			strncat(buf, table[*inp - 128], sizeof(buf) - strlen(buf) - 1);
		} else {
			letter[0] = *inp;
			strncat(buf, letter, sizeof(buf) - strlen(buf) - 1);
		}
	}
  
	return buf;
}

u_char *convert_cp1250(u_char *buf, int alloc)
{
	return char_table_conv(alloc ? (u_char*) xstrdup(buf) : buf, table_cp_iso);
}

u_char *convert_plain(u_char *inp, int charset, int alloc)
{
	switch (charset) {
		case 0:
			return char_table_conv(alloc ? (u_char*) xstrdup(inp) : inp, table_iso_plain);
		case 1:
			return inp;
		case 2:
		case 3:
			return string_table_conv(inp, table_unicode);
	}
	
	return NULL;
}

u_char *convert_phonetic(u_char *inp, int charset, int alloc)
{
	switch (charset) {
		case 0:
			return char_table_conv(string_table_conv(inp, table_phonetic_iso), table_iso_plain);
		case 1:
			return string_table_conv(inp, table_phonetic_iso);
		case 2:
		case 3:
			return string_table_conv(inp, table_unicode);
	}
	
	return NULL;
}
