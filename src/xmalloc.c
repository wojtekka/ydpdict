/*
 *  ydpdict
 *  (C) Copyright 2001-2004 Wojtek Kaniewski <wojtekka@toxygen.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydpconfig.h"
#include "ydpconvert.h"

extern int showerror(const u_char *msg);

void *xcalloc(size_t nmemb, size_t size)
{
	void *tmp = calloc(nmemb, size);

	if (!tmp)
		showerror(_("Brak pamiêci."));

	return tmp;
}

void *xmalloc(size_t size)
{
	void *tmp = malloc(size);

	if (!tmp)
		showerror(_("Brak pamiêci."));

	/* na wszelki wypadek wyczy¶æ bufor */
	memset(tmp, 0, size);
	
	return tmp;
}

void xfree(void *ptr)
{
	if (ptr)
		free(ptr);
}

void *xrealloc(void *ptr, size_t size)
{
	void *tmp = realloc(ptr, size);

	if (!tmp)
		showerror(_("Brak pamiêci."));

	return tmp;
}

char *xstrdup(const char *s)
{
	char *tmp;

	if (!s)
		return NULL;

	if (!(tmp = strdup(s)))
		showerror(_("Brak pamiêci."));

	return tmp;
}
