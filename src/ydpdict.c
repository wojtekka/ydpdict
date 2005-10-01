/*
	ydpdict
	(c) 1998-2003 wojtek kaniewski <wojtekka@irc.pl>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
			          
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
			                         
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include "ydpcore.h"
#include "ydpconfig.h"
#include "ydpsound.h"
#include "ydpconvert.h"

#define DEFDICT_AP "dict100.dat"
#define DEFINDEX_AP "dict100.idx"
#define DEFDICT_PA "dict101.dat"
#define DEFINDEX_PA "dict101.idx"

/* podstawowe zmienne programu */
unsigned char input[64];
char *def = NULL;
int fw, menu = 0, pos = 0, exact = 1, defmark = 0, defline = 0;
int defsize, defupd = 1, color_text, color_cf1, color_cf2, parse_rtf = 1;
int xsize, ysize, ctrlk = 0;

/* okienka ncurses */
WINDOW *wordwin, *defwin, *headwin, *splitwin;

/* deklaracja pó¼niej opisanych funkcji */
void showerror(char *msg);
void showmenu(int pos, int menu);
void onsignal();
void findword2();
void updateall();
int showdef(char *def, int first);
int read_config();
int ischar(unsigned char ch);
void sigsegv();
void sigresize();
void sigterm();
void redrawdef();
void preparewins();
void change_dict(int pl);

/* do dzie³a panowie... */
int main(int argc, char **argv)
{
	int ch;

	/* na mój sygna³... */
	signal(SIGSEGV, sigsegv);
	signal(SIGWINCH, sigresize);
	signal(SIGTERM, sigterm);
	signal(SIGINT, sigterm);

	/* wczytaj konfiguracjê (przed inicjalizacj± ncurses) */
	read_config(argc, argv);
	if (charset == 3)
		puts((char *)putchar);

	/* inicjalizacja ncurses */
	initscr();
	xsize = stdscr->_maxx + 1;
	ysize = stdscr->_maxy + 1;
	noecho();

	/* je¶li chcemy kolorków, to je przygotuj */
	if (use_color && has_colors()) {
		start_color();
		init_pair(1, config_text & 127, COLOR_BLACK);
		init_pair(2, config_cf1 & 127, COLOR_BLACK);
		init_pair(3, config_cf2 & 127, COLOR_BLACK);
		color_text = COLOR_PAIR(1) | (config_text & A_BOLD);
		color_cf1 = COLOR_PAIR(2) | (config_cf1 & A_BOLD);
		color_cf2 = COLOR_PAIR(3) | (config_cf2 & A_BOLD);
	} else {
		color_text = A_NORMAL;
		color_cf1 = A_NORMAL;
		color_cf2 = A_NORMAL;
	}

	/* zrób co¶ z okienkami */
	preparewins();

	/* ...komunikat, ze co¶ siê dzieje */
	waddstr(defwin, _("Proszê czekaæ, trwa ³adowanie s³ownika...\n"));

	/* oczywi¶cie wy¶wietl */
	updateall();

	/* teraz otwórz s³ownik */
	if (!opendict(filespath, dict_ap ? DEFINDEX_AP : DEFINDEX_PA, dict_ap ? DEFDICT_AP : DEFDICT_PA)) switch (ydperror) {
		case YDP_OUTOFMEMORY:
			showerror(_("Brak pamiêci."));
		case YDP_CANTOPENIDX:
			showerror(_("Nie mo¿na otworzyæ pliku indeksowego."));
		case YDP_CANTOPENDEF:
			showerror(_("Nie mo¿na otworzyæ pliku z definicjami."));
		case YDP_INVALIDFILE:
			showerror(_("B³±d podczas czytania plików."));
	}
	
	/* i do dzie³a! */
	do {
		redrawdef();
		ch = wgetch(wordwin);
		switch(ch) {
			case 9: /* TAB */
				defmark = (defmark) ? 0 : 1;
				break;
			case KEY_F(2):
				if (playsample(pos + menu) < 1) beep();
				break;
			case KEY_F(1):
				def = strdup(_("\
{\\b ydpdict-" VERSION "\\line\\cf1(c) 1998-2003 by wojtek kaniewski}\
\\par\\pard{\
hmm... tak w³a¶ciwie, to nie wiem co mia³bym tutaj wrzuciæ. wystarczy \
powiedzieæ, ¿e klawisz TAB prze³±cza okna, kursorami oraz PgUp i PgDn \
mo¿na przewijaæ zarówno listê s³ów, jak i definicjê s³owa. klawisz F2 \
powoduje odtworzenie próbki d¼wiêkowej, a F3 i F4 zmieniaj± odpowiednio \
s³ownik na angielsko-polski i polsko-angielski.\
}\\par\\pard{\
kontakt z autorem: {\\b wojtekka@irc.pl} \
najnowsze wersje s± dostêpne pod adresem {\\b ftp://dev.null.pl/pub/}\
}"));
				break;
			case KEY_F(3):
			case '<':
				change_dict(0);
				defline = 0;
				defupd = 1;
				break;
			case KEY_F(4):
			case '>':
				change_dict(1);
				defline = 0;
				defupd = 1;
				break;
			case KEY_UP:
				if (defmark) {
					if (defline > 0)
						defline--;
				} else {
					if (menu > 0)
						menu--;
					else
						if (pos > 0)
							pos--;
					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_PPAGE:
			  if (defmark) {
	  if (defline > ysize - 4) defline -= ysize - 3; else defline = 0;
	} else {
			    if (menu > 0) menu = 0; else if (pos > ysize - 5) pos -= ysize - 4; else pos = 0;
	  defline = 0;
	  defupd = 1;
	}
	break;
			case KEY_DOWN:
			  if (defmark) {
	  if (defline < defsize - (ysize - 3)) defline++;
	} else {
			    if (menu < ysize - 5) menu++; else if (pos < wordcount - (ysize - 4)) pos++;
	  defline = 0;
	  defupd = 1;
	}
	break;
			case KEY_NPAGE:
			  if (defmark) {
	  if (defline < defsize - (ysize - 3) * 2 - 1) defline += ysize - 3; else defline = defsize - (ysize - 3);
	} else {
			    if (menu < ysize - 5) menu = ysize - 5; else if (pos < wordcount - 39) pos += ysize - 4; else pos = wordcount - (ysize - 4);
	  defline = 0;
	  defupd = 1;
	}
	break;
			case KEY_F(9):
			  parse_rtf = parse_rtf ? 0 : 1;
	break;
			case KEY_BACKSPACE:
			case 127:
			case 8:
	if (defmark) break;
			  if (input[0]) input[strlen(input) - 1] = 0;
	findword2();
			  defupd = 1;
	break;
			case 11: /* ^K */
			  ctrlk = 2;
	break;
			case 21: /* ^U */
			  strcpy(input, "\0");
	break;
			case 24: /* ^X */
			  showerror(ctrlk ? _("Hmm... Joe? Nie znam tego pana...") : _("E---- (Emacs sucks! pico forever!!!)"));
	break;
			default:
			  if ((ch == 'x' || ch == 'X') && ctrlk) showerror(_("Hmm... Joe? Nie znam tego pana..."));
			  if (strlen(input) > 17 || !ischar(ch)) break;
	defmark=0;
	defline=0;
	input[strlen(input) + 1] = 0;
	input[strlen(input)] = (unsigned char) ch;
	if (!strcmp(&input[strlen(input) - 2], ":q") || !strcmp(&input[strlen(input)-3], ":wq")) showerror(_("E--- (Emacs sucks! vi forever!!!)"));
			  findword2();
			  defupd = 1;
	break;
		}
		if (ctrlk)
			ctrlk--;
	} while (ch != 27);
	
	showerror(NULL);
	return -1;
}

/* zakoñczenie killem */
void sigterm()
{
	showerror(NULL);
}

/* od¶wie¿a definicjê */
void redrawdef()
{
	if (defupd) {
		if (def)
			free(def);
		def = readdef(pos + menu);
		defupd = 0;
	}
	showmenu(pos, menu);
	defsize = showdef(def, defline);
	curs_set((defmark) ? 0 : 1);
	wattrset(splitwin, A_BOLD);
	mvwprintw(splitwin, ysize / 2, 0, (defmark) ? "-->" : "<--");
	updateall();
}

/* przygotowuje okienka */
void preparewins()
{
	int x;

	/* je¶li ju¿ istnia³y, to znaczy ¿e mamy resize */
	if (wordwin) {
		delwin(wordwin);
		delwin(defwin);
		delwin(headwin);
		delwin(splitwin);
	}
	
	/* utwórz co trzeba */
	wordwin = newwin(ysize - 3, 20, 2, 2);
	defwin = newwin(ysize - 3, xsize - 29, 2, 27);
	headwin = newwin(1, xsize, 0, 0);
	splitwin = newwin(ysize - 1, 4, 1, 23);
	
	/* teraz je przygotuj */
	keypad(wordwin, 1);
	keypad(defwin, 1);
	werase(wordwin);
	werase(defwin);
	werase(headwin);
	werase(splitwin);

	/* narysuj cudown± pionow± liniê */
	for (x = 0; x < ysize; x++) {
		switch (charset) {
			case 0:
				mvwaddstr(splitwin, x, 1, "|");
				break;
			case 1:
				mvwaddch(splitwin, x, 1, ACS_VLINE);
				break;
			default: /* unikod */
				mvwaddstr(splitwin, x, 1, "â”‚");
		}
	}

	/* teraz piêkny nag³ówek */
	wattrset(headwin, A_REVERSE);
	for (x = 0; x < xsize; x++)
		waddch(headwin, ' ');
	mvwaddstr(headwin, 0, 1, HEADER_NAME);
	mvwaddstr(headwin, 0, xsize - strlen(HEADER_COPYRIGHT) - 1, HEADER_COPYRIGHT);
}

/* rozszerzanie okienka? */
void sigresize()
{
	initscr();
	xsize = stdscr->_maxx + 1;
	ysize = stdscr->_maxy + 1;
	preparewins();
	redrawdef();
}

/* czy podany znaczek da siê wy¶wietliæ i wprowadziæ z klawiatury? */
int ischar(unsigned char ch)
{
	return (ch > 31 && ch < 128) || strchr("±æê³ñó¶¿¼¡ÆÊ£ÑÓ¦¯¬", ch);
}

#define is_visible(x) (ypos >= first && ypos < (ysize - 3) + first) ? x : ""
#define conv(x, y) (phon) ? (char*) convert_phonetic(x, y, 0) : (char*) convert_plain(x, y, 0)

int showdef(char *def, int first)
{
	int attr = color_text, attrs[16], level = 0, lastsp = 1, xpos = 0;
	int phon = 0, lp = 0, dispword, newline_, newattr, lastnl = 0;
	int ypos = 0, margin = 0, tp,newphon;
	char token[64],	line[80];

	/* wyczy¶æ okienko */
	werase(defwin);

	/* debug */ if (parse_rtf) { /* debug */

	/* do parsingu, gotowi, start! */
	while (*def) {
		dispword = 0;
		newline_ = 0;
		newattr = attr;
		newphon = phon;
		
		switch(*def) {
			case '{':
				if (level < 16)
					attrs[level++] = attr;
				dispword = 1;
				break;

			case '\\':
				def++;
				tp = 0;
				
				while ((*def >= 'a' && *def <= 'z') || (*def >='0' && *def <= '9'))
					token[tp++] = *def++;
				
				token[tp] = 0;
				
				if (*def == ' ')
					def++;
				
				if (!lastnl) {
					if (!strcmp(token, "par") || !strcmp(token, "line"))
						newline_ = 1;
					margin = 0;
				}
				
				if (!strcmp(token, "pard")) {
					newline_ = 1;
					margin = 0;
				}
				
				if (!strncmp(token, "sa", 2)) {
					margin = 1;
					wprintw(defwin, is_visible("   "));
					xpos = 3;
				}
				
				if (!strcmp(token, "b"))
					newattr |= A_BOLD;
				if (!strcmp(token, "cf0"))
					newattr = color_text;
				if (!strcmp(token, "cf1"))
					newattr = color_cf1;
				if (!strcmp(token, "cf2"))
					newattr = color_cf2;
				if (!strcmp(token, "cf5"))
					newattr = color_text;
				if (token[0] == 'f')
					newphon = 0;
				if (!strcmp(token, "f1"))
					newphon = 1;
				if (!strcmp(token, "qc"))
					newattr |= 0x8000; /* nie wy¶wietlaæ */
				
				if (!strcmp(token, "super")) {
					line[lp++] = '^';
					line[lp] = 0;
				}
				
				def--;
				dispword = 1;
				break;
				
			case '}':
				if (!level)
					break;
				newattr = attrs[--level];
				dispword = 1;
				newphon = 0;
				break;
			default:
				if (attr & 0x8000)
					break;
				wattrset(defwin, attr);
				lastnl = 0;
				
				switch (*def) {
					case ' ':
						if (lastsp)
							break;
						dispword = 1;
						lastsp = 1;
						if (!lp) {		/* baaardzo na oko³o :( */
							line[0] = ' ';
							line[1] = 0;
							lp = 1;
							lastsp = 0;
						}
						break;
						
					default:
						if (*def == 0x7f)
							(*def)--;
						line[lp++] = *def;
						line[lp] = 0;
						lastsp = 0;
				}
		}
		
		def++;

		if (dispword && lp) {
			if (50 - xpos < lp) {
				wprintw(defwin, is_visible("\n"));
				ypos++;
				if (margin)
					wprintw(defwin, is_visible("   "));
				waddstr(defwin, is_visible(conv(line, charset)));
				xpos = (margin) ? 3 : 0 + strlen(line);
			} else {
				waddstr(defwin, is_visible(conv(line, charset)));
				xpos += strlen(line);
			}
			if (lastsp && xpos != 50) {
				wprintw(defwin, is_visible(" "));
				xpos++;
			}
			lp = 0;
		}
		
		if (newline_ && !(attr & 0x8000)) {
			wprintw(defwin, is_visible("\n"));
			ypos++;
			xpos = (margin) ? 3 : 0;
			if (margin)
				wprintw(defwin, is_visible("   "));
			lastsp = 1;
			lastnl = 1;
		}
		attr = newattr;
		phon = newphon;
	}

	if (lp) {
		if (50 - xpos < lp) {
			wprintw(defwin, is_visible("\n"));
			ypos++;
		}
		waddstr(defwin, is_visible(conv(line, charset)));
	}
	ypos++;

	/* debug */ } else wprintw(defwin, def); /* debug */
	
	return ypos;
}

/* update wszystkich okienek */
void updateall()
{
	wnoutrefresh(headwin);
	wnoutrefresh(splitwin);
	wnoutrefresh(defwin);
	wnoutrefresh(wordwin);
	doupdate();
}

/* odszukuje w s³owniku s³owo i od razu na nie wskazuje */
void findword2()
{
	int x = findword(input);
	
	exact = (x == -1) ? 0 : 1;
	if (x != -1) {
		pos = x;
		menu = 0;
	}
	if (pos > wordcount - (ysize - 4)) {
		pos = wordcount - (ysize - 4);
		menu = x - pos;
	}
}

/* pokazuje cudowne menu */
void showmenu(int pos, int menu) {
	char buf[32];
	int y;

	werase(wordwin);
	
	for (y = 0; y < (ysize - 4); y++) {
		wattrset(wordwin, y == menu ? A_REVERSE : A_NORMAL);
		mvwprintw(wordwin, y + 1, 0, "                    ");
		mvwprintw(wordwin, y + 1, 1, convert_plain(strncpy(buf, words[pos + y], 32), charset, 0));
	}
	
	wattrset(wordwin, exact ? A_BOLD : A_NORMAL);
	mvwprintw(wordwin, 0, 0, "[__________________]");
	strcpy(buf, input);
	mvwprintw(wordwin, 0, 1, convert_plain(strncpy(buf, input, 32), charset, 0));
	wattrset(wordwin, A_NORMAL);
}

/* zamyka ncurses i wywala komunikat o b³êdzie */
void showerror(char *msg)
{
	werase(stdscr);
	wnoutrefresh(stdscr);
	doupdate();
	curs_set(1);
	endwin();
	if (msg)
		fprintf(stderr, "%s\n\n", msg);
	closedict();
	if (charset == 3)
		puts((char *)putchar);
	exit(msg ? 1 : 0);
}

/* na wszelki wypadek */
void sigsegv()
{
	signal(SIGSEGV, SIG_IGN);
	showerror(_("Naruszenie ochrony pamiêci (skontaktuj siê z autorem programu)"));
}

/* Zmienia s³ownik */
void change_dict(int pl)
{
	closedict();

	if (!opendict(filespath, (pl) ? DEFINDEX_PA : DEFINDEX_AP, pl ? DEFDICT_PA : DEFDICT_AP)) {
		switch (ydperror) {
			case YDP_OUTOFMEMORY:
				showerror(_("Brak pamiêci."));
			case YDP_CANTOPENIDX:
				showerror(_("Nie mo¿na otworzyæ pliku indeksowego."));
			case YDP_CANTOPENDEF:
				showerror(_("Nie mo¿na otworzyæ pliku z definicjami."));
			case YDP_INVALIDFILE:
				showerror(_("B³±d podczas czytania plików."));
		}
	}

	defline = 0;
	defupd = 1;
	menu = 0;
	pos = 0;
	updateall();
	memset(input, 0, sizeof(input));
}
