/*
 *  ydpdict
 *  (c) 1998-2004 wojtek kaniewski <wojtekka@irc.pl>
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
#include <curses.h>
#include <fcntl.h>
#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydpconfig.h"
#include "ydpconvert.h"
#include "xmalloc.h"

#ifdef HAVE_GETOPT_LONG
/* informacje dla getopt */
static struct option const longopts[] = {
	{ "help", no_argument, 0, 'h' },
	{ "version", no_argument, 0, 'v' },
	{ "pol", no_argument, 0, 'p' },
	{ "ang", no_argument, 0, 'a' },
	{ "pol-niem", no_argument, 0, 'P' },
	{ "niem-pol", no_argument, 0, 'D' },
	{ "path", required_argument, 0, 't' },
	{ "cdpath", required_argument, 0, 'c' },
	{ "nopl", no_argument, 0, 'n' },
	{ "iso", no_argument, 0, 'i' },
	{ "unicode", no_argument, 0, 'u' },
	{ "unicodeset", no_argument, 0, 'U' },
	{ "player", required_argument, 0, 'P' },
	{ "word", required_argument, 0, 'w'},
	{ 0, 0, 0, 0 }
};
#endif /* HAVE_GETOPT_LONG */

/* instrukcja u¿ycia */
void usage(const char *argv0) {
	printf("\
U¿ycie: %s [OPCJE]\n\
  -a, --ang             uruchamia s³ownik angielsko-polski (domy¶lne)\n\
  -p, --pol             uruchamia s³ownik polsko-angielski\n\
  -d, --niem-pol        uruchamia s³ownik niemiecko-polski\n\
  -o, --pol-niem        uruchamia s³ownik polsko-niemiecki\n\
  -n, --nopl            wy³±cza wy¶wietlanie polskich liter\n\
  -i, --iso		wy¶wietla polskie literki w standardzie ISO-8859-2\n\
  -u, --unicode		wy¶wietla polskie literki u¿ywaj±c unikodu\n\
  -U, --unicodeset	prze³±cza konsolê w tryb unikodu na czas dzia³ania\n\
  -f, --path=¦CIE¯KA    podaje ¶cie¿kê do plików danych\n\
  -c, --cdpath=¦CIE¯KA  podaje ¶cie¿kê do p³yty CD\n\
  -P, --player=¦CIE¯KA  podaje ¶cie¿kê do odtwarzacza plików WAV\n\
  -w, --word=S£OWO      uruchamia s³ownik i t³umaczy podane s³owo\n\
      --version		wy¶wietla wersjê programu\n\
  -h, --help		wy¶wietla ten tekst\n\
\n", argv0);
}

/* jakie¶tam zmienne */
char *e_labels[] = E_LABELS;
void *e_vals[] = E_VALS;
char *color_defs[] = COLOR_DEFS;
int color_vals[] = COLOR_VALS;

/* wczytuje konfiguracjê z pliku, a pó¼niej z argumentów wywo³ania */
int read_config(int argc, char **argv)
{
	u_char line[256], *par;
	int optc, l = 0, x, y;
	FILE *f;

	/* warto¶ci pocz±tkowe */
	filespath = "./";
	use_color = 1;
	charset = 1;
	dict = 1;
	config_text = COLOR_WHITE;
	config_cf1 = COLOR_CYAN | A_BOLD;
	config_cf2 = COLOR_GREEN | A_BOLD;

	/* sprawd¼, czy plik istnieje */
	snprintf(line, sizeof(line), "%s/%s", getenv("HOME"), CONFIGFILE_CWD1);
	f = fopen(line, "r");
	
	if (!f) {
		snprintf(line, sizeof(line), "%s/%s", getenv("HOME"), CONFIGFILE_CWD2);
		f = fopen(line, "r");
	}
	
	if (!f)
		f = fopen(CONFIGFILE_GLOBAL, "r");
	
	if (!f)
		return -1;
  
	/* ka¿d± liniê z osobna */
	while (fgets(line, sizeof(line), f)) {
		/* obrób liniê tak, ¿eby¶my nie dostawali ¶mieci */
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		if (line[strlen(line) - 1] == '\r')
			line[strlen(line) - 1] = '\0';
		if (line[0] == '#' || line[0] == '\0')
			continue;
		l++;

		/* sprawd¼, czy co¶ pasuje do zadeklarowanych w³a¶ciwo¶ci */
		for (x = 0; e_labels[x]; x++) {
			if (strncasecmp(line, &e_labels[x][2], strlen(e_labels[x]) - 2))
				continue;

			par = &line[strlen(&e_labels[x][2]) + 1];
			
			switch (e_labels[x][0]) {
				/* jaki¶ ³adny kolorek */
				case 'c':
					for (y = 0; color_defs[y]; y++) {
						if (!strcasecmp(par, color_defs[y]))
							*(int*)(e_vals[x]) = color_vals[y];
					}
					break;
					
				/* warto¶æ bool'owska: on lub off */
				case 'b':
					if (!strncasecmp(par, "On", 2))
						*(int*)(e_vals[x]) = TRUE;
					if (!strncasecmp(par, "Of", 2))
						*(int*)(e_vals[x]) = FALSE;
					break;
					
				/* warto¶æ ci±gu */
				case 's':
					*(char**)e_vals[x] = xstrdup(par);
					break;

				/* liczba */
				case 'i':
					*(int*)(e_vals[x]) = atoi(par);
					break;

				/* zestaw znaków */
				case 'h':
					if (!strncasecmp(par, "No", 2))
						charset = 0; /* bez polskich */
					if (!strncasecmp(par, "ISO", 3))
						charset = 1; /* iso-8859-2 */
					if (!strcasecmp(par, "Unicode"))
						charset = 2; /* unikod */
					if (!strcasecmp(par, "UnicodeSet"))
						charset = 3; /* unikod-2 */
					break;
      			}

      			break;
		}

		if (!e_labels[x]) {
			fprintf(stderr, "B£¡D: plik konfiguracyjny, linia %d: %s\n", l, line);
			exit(1);
		}
	}

	fclose(f);
  
#ifdef HAVE_GETOPT_LONG
	while ((optc = getopt_long(argc, argv, "hvVpaodf:c:niuUw:", longopts, (int*) 0)) != -1) {
#else
	while ((optc = getopt(argc, argv, "hvVpaodf:c:niuUw:")) != -1) {
#endif
		switch(optc) {
			case 'h':
				usage(argv[0]);
				exit(0);
			case 'v':
			case 'V':
				printf("ydpdict-" VERSION "\n");
				exit(0);
			case 'p':
				dict = 0;
				break;
			case 'a':
				dict = 1;
				break;
			case 'o':
				dict = 2;
				break;
			case 'd':
				dict = 3;
				break;
			case 'n':
				charset = 0;
				break;
			case 'i':
				charset = 1;
				break;
			case 'u':
				charset = 2;
				break;
			case 'U':
				charset = 3;
				break;
			case 'f':
				filespath = xstrdup(optarg);
				break;
			case 'c':
				cdpath = xstrdup(optarg);
				break;
			case 'P':
				player = xstrdup(optarg);
				break;
			case 'w':
				word = xstrdup(optarg);
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
	}

	return 0;
}
