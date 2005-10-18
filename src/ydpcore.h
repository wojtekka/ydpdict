/*
 *  ydpdict
 *  (c) 1998-2005 wojtek kaniewski <wojtekka@irc.pl>
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

#ifndef __YDPCORE_H
#define __YDPCORE_H

#define YDP_NONE 0
#define YDP_CANTOPENIDX 1
#define YDP_CANTOPENDEF 2
#define YDP_INVALIDFILE 3

unsigned int *indexes;	/* tablica z indeksami definicji w pliku indeksowym */
u_char **words;		/* tablica z list± s³ów */
unsigned short wordcount;	/* ilo¶æ s³ów */
FILE *fd, *fi;		/* strumienie */
int ydperror;		/* kod b³êdu */

int opendict(const u_char *path, const u_char *index, const u_char *def);
void closedict();
u_char *readdef(int i);
int findword(const u_char *word);

#endif /* __YDPCORE_H */
