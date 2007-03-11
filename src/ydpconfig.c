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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_NCURSESW_NCURSES_H
#  include <ncursesw/ncurses.h>
#else
#  ifdef HAVE_CURSES_H
#    include <curses.h>
#  endif
#endif
#include <fcntl.h>
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydpconfig.h"
#include "xmalloc.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif

char *config_path = NULL;
char *config_cdpath = NULL;
char *config_player = NULL;
char *config_word = NULL;
int config_dict = 1;
int config_color = 1;
int config_transparent = 0;
int config_text = COLOR_WHITE;
int config_cf1 = COLOR_CYAN | A_BOLD;
int config_cf2 = COLOR_GREEN | A_BOLD;

#ifdef HAVE_GETOPT_LONG

static struct option const longopts[] = {
	{ "help", no_argument, 0, 'h' },
	{ "version", no_argument, 0, 'V' },
	{ "eng", no_argument, 0, 'e' },
	{ "pol", no_argument, 0, 'p' },
	{ "pol-ger", no_argument, 0, 'o' },
	{ "ger-pol", no_argument, 0, 'g' },
	{ "path", required_argument, 0, 't' },
	{ "cdpath", required_argument, 0, 'c' },
	{ "player", required_argument, 0, 'P' },
	{ "word", required_argument, 0, 'w'},
	{ "audio", required_argument, 0, 'A'},
	{ 0, 0, 0, 0 }
};

#endif /* HAVE_GETOPT_LONG */

/**
 * \brief Prints usage instructions
 * 
 * \param argv0 argv[0] from main()
 */
void usage(const char *argv0) {
	printf(_(
"Usage: %s [OPTIONS]\n"
"  -e, --eng             start with English-Polish dictionary (default)\n"
"  -p, --pol             start with Polish-English dictionary\n"
"  -g, --ger-pol         start with German-Polish dictionary\n"
"  -o, --pol-ger         start with Polish-German dictionary\n"
"  -f, --path=PATH       set database path\n"
"  -c, --cdpath=PATH     set CD-ROM path\n"
"  -P, --player=PATH     set sound player path\n"
"  -w, --word=WORD       start with selected WORD's definition\n"
"  -A, --audio=DEVICE    set default audio device\n"
"  -V, --version         print program version\n"
"  -h, --help            print this message\n"
"\n"), argv0);
}

enum config_entry_type {
	ENTRY_COLOR = 1,
	ENTRY_BOOL,
	ENTRY_STRING,
	ENTRY_INT
};

struct config_entry {
	int type;
	const char *label;
	void *ptr;
} entries[] = {
	{ ENTRY_COLOR, "Color", &config_text },
	{ ENTRY_COLOR, "DefinitionColor", &config_cf1 },
	{ ENTRY_COLOR, "InformationColor", &config_cf2 },
	{ ENTRY_BOOL, "UseColor", &config_color },
	{ ENTRY_STRING, "Path", &config_path },
	{ ENTRY_STRING, "CDPath", &config_cdpath },
	{ ENTRY_STRING, "Player", &config_player },
	{ ENTRY_STRING, "AudioDevice", &config_audio },
	{ ENTRY_INT, "DefaultDictionary", &config_dict },
	{ ENTRY_BOOL, "UseTransparent", &config_transparent },
	{ 0, NULL, NULL }
};

struct color_entry {
	const char *name;
	int value;
} colors[] = {
	{ "Black", COLOR_BLACK },
	{ "Red", COLOR_RED },
	{ "Greed", COLOR_GREEN },
	{ "Brown", COLOR_YELLOW },
	{ "Blue", COLOR_BLUE },
	{ "Magenta", COLOR_MAGENTA },
	{ "Cyan", COLOR_CYAN },
	{ "White", COLOR_WHITE },
	{ "Gray", COLOR_WHITE | A_DIM },
	{ "LightRed", COLOR_RED },
	{ "LightGreed", COLOR_GREEN | A_BOLD },
	{ "Yellow", COLOR_YELLOW | A_BOLD },
	{ "LightBlue", COLOR_BLUE | A_BOLD },
	{ "LightMagenta", COLOR_MAGENTA | A_BOLD },
	{ "LightCyan", COLOR_CYAN | A_BOLD },
	{ "LightWhite", COLOR_WHITE | A_BOLD },
	{ NULL, 0 }
};

/**
 * \brief Reads configuration from file, then handles program arguments
 *
 * \param argc argc passed from main()
 * \param argv argv passed from main()
 *
 * \return 0 on success, -1 on failure
 */
int read_config(int argc, char **argv)
{
	char buf[4096], *home;
	int line = 0, optc;
	FILE *f = NULL;

	/* Check if any of the config files exist */

	home = getenv("HOME");

	if (home) {
		snprintf(buf, sizeof(buf), "%s/%s", home, CONFIGFILE_CWD1);
		f = fopen(buf, "r");
	
		if (!f) {
			snprintf(buf, sizeof(buf), "%s/%s", home, CONFIGFILE_CWD2);
			f = fopen(buf, "r");
		}
	}
	
	if (!f)
		f = fopen(CONFIGFILE_GLOBAL, "r");
	
	if (!f)
		return -1;
  
	/* Parse every line */

	while (fgets(buf, sizeof(buf), f)) {
		int len, i;

		line++;

		len = strlen(buf);

		if (len && buf[len - 1] == '\n')
			buf[--len] = 0;
		if (len && buf[len - 1] == '\r')
			buf[--len] = 0;
		if (!len || buf[0] == '#')
			continue;

		for (i = 0; entries[i].label; i++) {
			const char *value;

			len = strlen(entries[i].label);

			if (strncasecmp(buf, entries[i].label, len) || !buf[len])
				continue;

			value = buf + len + 1;
			
			switch (entries[i].type) {
				case ENTRY_COLOR:
				{
					int j;
					
					for (j = 0; colors[j].name; j++) {
						if (!strcasecmp(value, colors[j].name))
							*(int*)(entries[i].ptr) = colors[j].value;
					}
					break;
				}
					
				case ENTRY_BOOL:
					if (!strncasecmp(value, "on", 2))
						*(int*)(entries[i].ptr) = 1;
					if (!strncasecmp(value, "off", 2))
						*(int*)(entries[i].ptr) = 0;
					break;
					
				case ENTRY_STRING:
					if (*(char**)(entries[i].ptr))
						xfree(*(char**)entries[i].ptr);
					*(char**)(entries[i].ptr) = xstrdup(value);
					break;

				case ENTRY_INT:
					*(int*)(entries[i].ptr) = atoi(value);
					break;
      			}

      			break;
		}

		if (!entries[i].label) {
			fprintf(stderr, _("Error in config file, like %d: %s\n"), line, buf);
			exit(1);
		}
	}

	fclose(f);
  
#ifdef HAVE_GETOPT_LONG
	while ((optc = getopt_long(argc, argv, "hvVpeogf:c:w:A:", longopts, (int*) 0)) != -1) {
#else
	while ((optc = getopt(argc, argv, "hvVpaogf:c:w:A:")) != -1) {
#endif
		switch(optc) {
			case 'h':
				usage(argv[0]);
				exit(0);

			case 'v':
			case 'V':
				printf("ydpdict-" VERSION "\n");
				exit(0);

			case 'a':
				config_dict = 0;
				break;

			case 'p':
				config_dict = 1;
				break;

			case 'g':
				config_dict = 2;
				break;

			case 'o':
				config_dict = 3;
				break;

			case 'f':
				if (config_path)
					xfree(config_path);
				config_path = xstrdup(optarg);
				break;

			case 'c':
				if (config_cdpath)
					xfree(config_cdpath);
				config_cdpath = xstrdup(optarg);
				break;

			case 'P':
				if (config_path)
					xfree(config_path);
				config_path = xstrdup(optarg);
				break;

			case 'w':
				if (config_word)
					xfree(config_word);
				config_word = xstrdup(optarg);
				break;

			case 'A':
				if (config_audio)
					xfree(config_audio);
				config_audio = xstrdup(optarg);
				break;

			default:
				usage(argv[0]);
				exit(1);
		}
	}

	if (!config_path)
		config_path = xstrdup(DEFAULT_PATH);
	
	if (!config_cdpath)
		config_cdpath = xstrdup(DEFAULT_CDPATH);
	
#ifdef DEFAULT_AUDIO
	if (!config_audio)
		config_audio = xstrdup(DEFAULT_AUDIO);
#endif
	
	return 0;
}

