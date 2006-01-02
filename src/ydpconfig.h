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

#ifndef __YDPCONFIG_H
#define __YDPCONFIG_H

#include <curses.h>

#define VERSION "0.66"
#define HEADER_NAME "ydpdict-" VERSION
#define HEADER_COPYRIGHT "(C) Copyright 1998-2006 Wojtek Kaniewski"
#define CONFIGFILE_GLOBAL "/etc/ydpdict.conf"
#define CONFIGFILE_CWD1 "ydpdict.conf"
#define CONFIGFILE_CWD2 ".ydpdictrc"

/* konfiguracja */
u_char *filespath, *cdpath, *player, *word;
int dict, charset, use_color, transparent, config_text, config_cf1, config_cf2;

/* g��wna funkcja */
int read_config(int argc, char **argv);

/* jakie s� wpisy w pliku konfiguracyjnym? c:kolor, b:boolean, s:tekst, h:charset */
#define E_LABELS { "c:Color", "c:DefinitionColor", "c:InformationColor", \
	"b:UseColor", "h:Charset", "s:Path", "s:CDPath", "s:Player", \
	"i:DefaultDictionary", "b:UseTransparent", 0 }
#define E_VALS { &config_text, &config_cf1, &config_cf2, &use_color, &charset, \
	&filespath, &cdpath, &player, &dict, &transparent, 0 }

/* sta�e okre�laj�ce, jak nazywa si� dany kolor */
#define COLOR_DEFS { "Black", "Red", "Green", "Brown", "Blue", "Magenta", \
	"Cyan", "White", "Gray", "LightRed", "LightGreen", "Yellow", \
	"LightBlue", "LightMagenta", "LightCyan", "LightWhite", 0 }
#define COLOR_VALS { COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW, \
	COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE, COLOR_WHITE \
	| A_DIM, COLOR_RED | A_BOLD, COLOR_GREEN | A_BOLD, COLOR_YELLOW \
	| A_BOLD, COLOR_BLUE | A_BOLD, COLOR_MAGENTA | A_BOLD, COLOR_CYAN \
	| A_BOLD, COLOR_WHITE | A_BOLD, 0 }

#endif /* __YDPCONFIG_H */
