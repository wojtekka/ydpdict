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

#ifndef __YDPCONFIG_H
#define __YDPCONFIG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define HEADER_NAME "ydpdict-" VERSION
#define HEADER_COPYRIGHT "(C) Copyright 1998-2007 Wojtek Kaniewski"
#define HELP_EMAIL "wojtekka@toxygen.net"
#define HELP_WEBSITE "http://toxygen.net/ydpdict/"
#define CONFIGFILE_GLOBAL SYSCONFDIR "/ydpdict.conf"
#define CONFIGFILE_CWD1 "ydpdict.conf"
#define CONFIGFILE_CWD2 ".ydpdictrc"

#define DEFAULT_PATH "."
#define DEFAULT_CDPATH "/media/cdrom"

#ifndef HAVE_LIBAO
#define DEFAULT_AUDIO "/dev/dsp"
#endif

#define DEFAULT_DAT_AP "dict100.dat"
#define DEFAULT_IDX_AP "dict100.idx"
#define DEFAULT_DAT_PA "dict101.dat"
#define DEFAULT_IDX_PA "dict101.idx"

#define DEFAULT_DAT_DP "dict200.dat"
#define DEFAULT_IDX_DP "dict200.idx"
#define DEFAULT_DAT_PD "dict201.dat"
#define DEFAULT_IDX_PD "dict201.idx"

#define INPUT_LEN 17

char *config_path;
char *config_cdpath;
char *config_player;
char *config_word;
char *config_audio;
int config_dict;
int config_color;
int config_transparent;
int config_text;
int config_cf1;
int config_cf2;

int read_config(int argc, char **argv);

#endif /* __YDPCONFIG_H */
