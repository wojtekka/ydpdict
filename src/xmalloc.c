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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif

extern int show_error(const char *msg);

/**
 * \bried calloc() with error handling
 *
 * \param nmemb member count
 * \param size member size
 * 
 * \return Allocated buffer
 */
void *xcalloc(size_t nmemb, size_t size)
{
	void *tmp = calloc(nmemb, size);

	if (!tmp)
		show_error(_("Out of memory"));

	return tmp;
}

/**
 * \bried malloc() with error handling and zeroing
 *
 * \param size buffer size
 * 
 * \return Allocated buffer
 */
void *xmalloc(size_t size)
{
	void *tmp = malloc(size);

	if (!tmp)
		show_error(_("Out of memory"));

	memset(tmp, 0, size);
	
	return tmp;
}

/**
 * \brief free() with error handling
 *
 * \param ptr pointer to be freed
 */
void xfree(void *ptr)
{
	if (ptr)
		free(ptr);
}

/**
 * \brief realloc() with error handling
 *
 * \param ptr pointer to be reallocated
 * \param size new size
 *
 * \return Allocated buffer
 */
void *xrealloc(void *ptr, size_t size)
{
	void *tmp = realloc(ptr, size);

	if (!tmp)
		show_error(_("Out of memory"));

	return tmp;
}

/**
 * \brief strdup() with error handling
 *
 * \param s string to be duplicated
 *
 * \return Allocated buffer
 */
char *xstrdup(const char *s)
{
	char *tmp;

	if (!s)
		return NULL;

	if (!(tmp = strdup(s)))
		show_error(_("Out of memory"));

	return tmp;
}
