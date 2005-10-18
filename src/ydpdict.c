/*
 *  ydpdict
 *  (c) 1998-2005 wojtek kaniewski <wojtekka@irc.pl>
 *                piotr domagalski <szalik@szalik.net>
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

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydpcore.h"
#include "ydpconfig.h"
#include "ydpsound.h"
#include "ydpconvert.h"
#include "xmalloc.h"

#define DEFDICT_AP "dict100.dat"
#define DEFINDEX_AP "dict100.idx"
#define DEFDICT_PA "dict101.dat"
#define DEFINDEX_PA "dict101.idx"

#define DEFDICT_DP "dict200.dat"
#define DEFINDEX_DP "dict200.idx"
#define DEFDICT_PD "dict201.dat"
#define DEFINDEX_PD "dict201.idx"

#define INPUTLEN 17

/* podstawowe zmienne programu */
u_char input[INPUTLEN + 1], *def = NULL;
int fw, menu = 0, menux = 0, pos = 0, exact = 1, defmark = 0, defline = 0;
int defsize, defupd = 0, color_text, color_cf1, color_cf2, parse_rtf = 1;
int xsize, ysize, resized_term = 0, ctrlk = 0;
int init;
int length;

/* okienka ncurses */
WINDOW *wordwin = NULL, *defwin = NULL, *headwin = NULL, *splitwin = NULL;

/* deklaracja pó¼niej opisanych funkcji */
void showerror(const u_char *msg);
void showmenu(int pos, int menu);
void findword2();
void updateall();
int showdef(u_char *def, int first);
int read_config();
int ischar(u_char ch);
void sigsegv();
void sigresize();
void resize();
void checksize();
void sigterm();
void redrawdef();
void preparewins();
void change_dict(int pl);

/* do dzie³a panie i panowie... */
int main(int argc, char **argv)
{
	int ch, bg;
	MEVENT m_event;

	/* na mój sygna³... */
	signal(SIGSEGV, sigsegv);
#ifdef SIGWINCH
	signal(SIGWINCH, sigresize);
#endif
	signal(SIGTERM, sigterm);
	signal(SIGINT, sigterm);
	signal(SIGHUP, sigterm);

	/* wczytaj konfiguracjê (przed inicjalizacj± ncurses) */
	read_config(argc, argv);
	if (charset == 3) {
		puts("\033%G");
		fflush(stdout);
	}

	/* inicjalizacja ncurses */
	initscr();
	noecho();
	cbreak();

#ifndef COLOR_DEFAULT
#  define COLOR_DEFAULT (-1)
#endif

	if (transparent) {
		bg = COLOR_DEFAULT;
		use_default_colors();
	} else
		bg = COLOR_BLACK;

	/* je¶li chcemy kolorków, to je przygotuj */
	if (use_color && has_colors()) {
		start_color();
		init_pair(1, config_text & 127, bg);
		init_pair(2, config_cf1 & 127, bg);
		init_pair(3, config_cf2 & 127, bg);
		color_text = COLOR_PAIR(1) | (config_text & A_BOLD);
		color_cf1 = COLOR_PAIR(2) | (config_cf1 & A_BOLD);
		color_cf2 = COLOR_PAIR(3) | (config_cf2 & A_BOLD);
	} else {
		color_text = A_NORMAL;
		color_cf1 = A_NORMAL;
		color_cf2 = A_NORMAL;
	}

	memset(input, 0, sizeof(input));

	init = 1;

	/* sprawd¼, czy siê zmie¶cimy */
	checksize();

	/* i zrób co¶ z okienkami */
	preparewins();

	/* za³aduj s³ownik */
	change_dict(dict);

	init = 0;

	mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);

	/* je¿eli poda³e¶ parametr word to wczytaj slowo*/
	if (word)	
		for (length = ((int) strlen(word)) - 1; length>=0; length--)
			ungetch(word[length]);
	else
		/* wy¶wietl pomoc po uruchomieniu */
		ungetch('?');
	
	/* i do dzie³a! */
	for (;;) {
		if (resized_term)
			resize();

		redrawdef();
		ch = wgetch(wordwin);

		switch (ch) {

#define __MOUSE_IN(window, event, correct1, correct2, correct3, correct4) (\
	 event.y > (window->_begy + correct1) && event.y < (window->_begy + window->_maxy + correct2) &&\
	 event.x > (window->_begx + correct3) && event.x < (window->_begx + window->_maxx + correct4))

#define isalpha_pl_PL(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || x == (u_char) '±' ||\
	x == (u_char) 'æ' || x == (u_char) 'ê' || x == (u_char) '³' || x == (u_char) 'ñ' ||\
	x == (u_char) 'ó' || x == (u_char) '¶' || x == (u_char) '¿' || x == (u_char) '¼' ||\
	x == (u_char) '¡' || x == (u_char) 'Æ' || x == (u_char) 'Ê' || x == (u_char) '£' ||\
	x == (u_char) 'Ñ' || x == (u_char) 'Ó' || x == (u_char) '¦' || x == (u_char) '¯' ||\
	x == (u_char) '¬')

			case KEY_MOUSE:
				if (getmouse(&m_event) == OK) {

					/* dwukrotne klikniêcie s³owa w opisie */
					if (m_event.bstate & BUTTON1_DOUBLE_CLICKED && __MOUSE_IN(defwin, m_event, -2, 1, -3, 2)) {
						u_char c, buf[INPUTLEN + 1];
						int i = 0, x = m_event.x - 27;

						c = (u_char) (mvwinch(defwin, m_event.y - 2, x) & A_CHARTEXT);

						if (!(isalpha_pl_PL(c) || c == '-'))
							break;

						/* szukamy pierwszej nie-litery */
						do {
							x--;
							c = (u_char) (mvwinch(defwin, m_event.y - 2, x) & A_CHARTEXT);
						} while ((isalpha_pl_PL(c) || c == '-') && x >= 0);

						/* i na prawo próbujemy uzbieraæ s³owo */
						do {
							x++;
							c = (u_char) (mvwinch(defwin, m_event.y - 2, x) & A_CHARTEXT);
							buf[i++] = c;
						} while ((isalpha_pl_PL(c) || c == '-') && x < defwin->_maxx);

						buf[--i] = 0;

						/* uzbierali¶my co¶ ? */
						if (strlen(buf)) {
							strncpy(input, buf, sizeof(input) - 1);
							menux = strlen(input);
							findword2();
							defupd = 1;
						}

						break;
					}

					if (m_event.bstate & BUTTON1_CLICKED || m_event.bstate & BUTTON1_DOUBLE_CLICKED) {
						/* wskazanie s³owa na li¶cie */
						if (__MOUSE_IN(wordwin, m_event, -2, 1, -1, 1) && m_event.y > 2) {
							menu = m_event.y - 3;
							memset(&input, 0, sizeof(input));
							menux = 0;
							defline = 0;
							defupd = 1;
						}

						/* copyright ;> */
						if (m_event.y == 0 && m_event.x > xsize - strlen(HEADER_COPYRIGHT) - 2) {
							ungetch(KEY_F(1));
							break;
						}

						/* przewijanie ekranu */
						if (m_event.y == ysize - 1) {
							ungetch(KEY_NPAGE);
							break;
						}

						if (m_event.y <= 1) {
							ungetch(KEY_PPAGE);
							break;
						}

					}

					if (m_event.bstate & BUTTON1_CLICKED) {
						/* zmiana ,,aktywnego'' okna */
						if (__MOUSE_IN(wordwin, m_event, -2, 1, -3, 2) && defmark)
							defmark = 0;
						if (__MOUSE_IN(defwin, m_event, -2, 1, -3, 2) && !defmark)
							defmark = 1;
					}
				}

				break;
#undef isalpha_pl_PL
#undef __MOUSE_IN

			case 10: /* Enter */
				if (defmark) {
					if (defline < defsize - (ysize - 3))
						defline++;
				} else {
					char *c = &input[strlen(input) - 1];
					while (*c == ' ')
						*c-- = 0;

					menux = strlen(input);

					findword2();
					defupd = 1;
				}
				break;
			case 27:
				/* ESC */
				if ((ch = wgetch(wordwin)) == ERR || ch == 27)
					showerror(NULL);
				break;
#ifdef KEY_RESIZE
			case KEY_RESIZE:
				resized_term = 1;
				break;
#endif
			case 9: /* TAB */
				defmark = (defmark) ? 0 : 1;
				break;
			case KEY_F(2):
			case '`':
				if (dict == 1 || dict == 3)
					if (playsample(pos + menu) < 1);

				break;
			case KEY_F(1):
			case '?':
				def = strdup(_("\
{\\b ydpdict-" VERSION "\\line\\cf1(c) 1998-2005 by wojtek kaniewski}\
\\par\\pard{\
}\
{\\line{\\cf2 F1} lub {\\cf2 ?} - pomoc}\
{\\line{\\cf2 Tab} - zmiana panelu}\
{\\line{\\cf2 strza³ka w dó³} lub {\\cf2 w górê} - przewijanie paneli wiersz po wierszu}\
{\\line{\\cf2 Page Up} lub {\\cf2 Page Down} - przewijanie paneli ekranami}\
{\\line{\\cf2 F2} lub {\\cf2 `} - odtworzenie wymowy wyrazu}\
{\\line{\\cf2 F3} lub {\\cf2 <} - s³ownik angielsko-polski}\
{\\line{\\cf2 F4} lub {\\cf2 >} - s³ownik polsko-angielski}\
{\\line{\\cf2 F5} lub {\\cf2 [} - s³ownik niemiecko-polski}\
{\\line{\\cf2 F6} lub {\\cf2 ]} - s³ownik polsko-niemiecki}\
{\\line{\\cf2 Ctrl-U} - usuniêcie wpisywanego s³owa}\
{\\line{\\cf2 Ctrl-L} - od¶wie¿enie okna}\
{\\line{\\cf2 strza³ka w lewo} lub {\\cf2 w prawo} - poruszanie siê w s³owie}\
{\\line{\\cf2 Enter} - zakoñczenie edycji s³owa}\
{\\line{\\cf2 Esc} lub {\\cf2 Ctrl-C} - zakoñczenie pracy z programem.}\
\\par\\pard{\
kontakt z autorem: {\\b wojtekka@irc.pl} \
najnowsze wersje s± dostêpne pod adresem {\\b http://toxygen.net/ydpdict/}\
}"));
				break;
			case KEY_F(3):
			case '<':
				if (dict != 0) {
					change_dict(0);
					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_F(4):
			case '>':
				if (dict != 1) {
					change_dict(1);
					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_F(5):
			case '[':
				if (dict != 2) {
					change_dict(2);
					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_F(6):
			case ']':
				if (dict != 3) {
					change_dict(3);
					defline = 0;
					defupd = 1;
				}
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
	  				if (defline > ysize - 4)
						defline -= ysize - 3;
					else
						defline = 0;
				} else {
					if (menu > 0)
						menu = 0;
					else if (pos > ysize - 5)
						pos -= ysize - 4;
					else pos = 0;

					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_DOWN:
				if (defmark) {
					if (defline < defsize - (ysize - 3))
						defline++;
				} else {
					if (menu < ysize - 5)
						menu++;
					else if (pos < wordcount - (ysize - 4))
						pos++;
					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_NPAGE:
				if (defmark) {
					if (defline < defsize - (ysize - 3) * 2 - 1)
						defline += ysize - 3;
					else
						defline = defsize - (ysize - 3);
				} else {
					if (menu < ysize - 5)
						menu = ysize - 5;
					else if (pos < wordcount - 39)
						pos += ysize - 4;
					else
						pos = wordcount - (ysize - 4);
					defline = 0;
					defupd = 1;
				}
				break;
			case KEY_F(9):
				parse_rtf = (parse_rtf) ? 0 : 1;
				break;
			case KEY_DC:
				if (!defmark) {
					if (menux < strlen(input)) {
						memmove(&input[menux], &input[menux + 1], strlen(&input[menux]) + 1);
						findword2();
						defupd = 1;
					} else
						beep();
				}
				break;
			case KEY_BACKSPACE:
			case 127:
			case 8:
				if (defmark) {
					if (defline > 0)
						defline--;
					break;
				} else {
					if (menux) {
						memmove(&input[menux - 1], &input[menux], strlen(&input[menux]) + 1);
						menux--;
						findword2();
						defupd = 1;
					} else
						beep();
				}
				break;
			case 12: /* ^L */
				/* przerysowanie wszystkiego od nowa */
				resized_term = 1;
				break;
			case 11: /* ^K */
				ctrlk = 2;
				break;
			case 21: /* ^U */
				memset(&input, 0, sizeof(input));
				menu = 0;
				menux = 0;
				pos = 0;
				defupd = 1;
				break;
			case 24: /* ^X */
				showerror(ctrlk ? _("Hmm... Joe? Nie znam tego pana...") : _("E---- (Emacs sucks! pico forever!!!)"));
				break;
			case KEY_HOME:
			case KEY_FIND:
				menux = 0;
				break;
			case KEY_END:
			case KEY_SELECT:
				menux = strlen(input);
				break;
			case KEY_LEFT:
				if (!menux || defmark)
					beep();
				else
					menux--;
				break;
 			case KEY_RIGHT:
				if (menux >= strlen(input) || defmark)
					beep();
				else
					menux++;
				break;
			default:
				if ((ch == 'x' || ch == 'X') && ctrlk)
					showerror(_("Hmm... Joe? Nie znam tego pana..."));

				if (!ischar(ch))
					break;

				if (defmark) {
					memset(input, 0, sizeof(input));
					menux = 0;
				}

				if (strlen(input) > INPUTLEN)
					break;

				if (menux < strlen(input))
					memmove(&input[menux + 1], &input[menux], strlen(&input[menux]) + 1);
				
				input[menux++] = (u_char) ch;

				if (!strcmp(&input[strlen(input) - 2], ":q") || !strcmp(&input[strlen(input) - 3], ":wq"))
					showerror(_("E--- (Emacs sucks! vi forever!!!)"));

				findword2();
				defupd = 1;
				defmark = 0;
				defline = 0;
				break;
		}

		if (ctrlk)
			ctrlk--;
	}
	
	showerror(NULL);
	return 0;
}

/* zakoñczenie killem lub CTRL-C */
void sigterm()
{
	showerror(NULL);
}

/* od¶wie¿a definicjê */
void redrawdef()
{
	if (defupd) {
		xfree(def);
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

	/* je¶li ju¿ istnia³y, to znaczy, ¿e mamy resize */
	if (wordwin || defwin || headwin || splitwin) {
		delwin(wordwin);
		delwin(defwin);
		delwin(headwin);
		delwin(splitwin);
	}
	
	/* utwórz, co trzeba */
	wordwin = newwin(ysize - 3, 20, 2, 2);
	defwin = newwin(ysize - 3, xsize - 29, 2, 27);
	headwin = newwin(1, xsize, 0, 0);
	splitwin = newwin(ysize - 1, 4, 1, 23);

	if (!wordwin || !defwin || !headwin || !splitwin)
		showerror(_("Brak pamiêci."));
	
	/* teraz je przygotuj */
	keypad(wordwin, TRUE);
	halfdelay(100);

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

	{
		const u_char *hname = HEADER_NAME, *hcopyright = HEADER_COPYRIGHT;

		for (x = 0; x < xsize; x++)
			waddch(headwin, ' ');

		/* na pocz±tku... */
		for (x = 1; x < xsize && *hname; x++)
			mvwaddch(headwin, 0, x, *hname++);

		/* ... i na koñcu */
		for (x = xsize - strlen(hcopyright) - 1; x < xsize && *hcopyright; x++)
			mvwaddch(headwin, 0, x, *hcopyright++);
	}

}

/* rozszerzanie okienka? */
void sigresize()
{
	resized_term = 1;
}

void resize()
{
	endwin();
	refresh();

	checksize();
	preparewins();

	resized_term = 0;
}

void checksize()
{
	int newx, newy, fake = 0;

	newx = stdscr->_maxx + 1;
	newy = stdscr->_maxy + 1;

	/* minimalne rozmiary */
	if (newx < 29)
		fake = newx = 29;
	if (newy < 4)
		fake = newy = 4;
	
	if (fake)
		resizeterm(newy, newx);

	xsize = newx;
	ysize = newy;
}

/* czy podany znaczek da siê wy¶wietliæ i wprowadziæ z klawiatury? */
int ischar(u_char ch)
{
	return (ch > 31 && ch < 128) || strchr("±æê³ñó¶¿¼¡ÆÊ£ÑÓ¦¯¬", ch);
}

#define is_visible(x) ((ypos >= first && ypos < (ysize - 3) + first) ? x : "")
#define conv(x, y) ((phon) ? (char*) convert_phonetic(x, y, 0) : (char*) convert_plain(x, y, 0))

int showdef(u_char *def, int first)
{
	int attr = color_text, attrs[16], level = 0, lastsp = 1, xpos = 0;
	int phon = 0, lp = 0, dispword, newline_, newattr, lastnl = 0;
	int ypos = 0, margin = 0, tp, newphon;
	u_char token[64], line[80];

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
				
				while ((*def >= 'a' && *def <= 'z') || (*def >= '0' && *def <= '9'))
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

	if (exact) {
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
	u_char buf[32];
	int y;

	werase(wordwin);
	
	for (y = 0; y < (ysize - 4); y++) {
		wattrset(wordwin, y == menu ? A_REVERSE : A_NORMAL);
		mvwprintw(wordwin, y + 1, 0, "                    ");
		mvwprintw(wordwin, y + 1, 1, convert_plain(strncpy(buf, words[pos + y], sizeof(buf) - 1), charset, 0));
	}
	
	wattrset(wordwin, exact ? A_BOLD : A_NORMAL);
	mvwprintw(wordwin, 0, 0, "[__________________]");
	mvwprintw(wordwin, 0, 1, convert_plain(strncpy(buf, input, sizeof(buf) - 1), charset, 0));
	wattrset(wordwin, A_NORMAL);

	wmove(wordwin, 0, menux + 1);
}

#define xdelwin(x) { if (x) delwin(x); }

/* zamyka ncurses i wywala komunikat o b³êdzie */
void showerror(const u_char *msg)
{
	xdelwin(wordwin);
	xdelwin(headwin);
	xdelwin(splitwin);
	xdelwin(defwin);

	werase(stdscr);
	wrefresh(stdscr);

	endwin();
	closedict();
	xfree(def);

	if (msg)
		fprintf(stderr, "%s\n\n", msg);

	if (charset == 3) {
		puts("\033%@");
		fflush(stdout);
	}

	exit(msg ? 1 : 0);
}

/* na wszelki wypadek */
void sigsegv()
{
	signal(SIGSEGV, SIG_IGN);
	showerror(_("Naruszenie ochrony pamiêci (skontaktuj siê z autorem programu)"));
}

/* wybiera s³ownik */
void change_dict(int new_dict)
{
	const char *idx, *dat;

	if (!init)
		closedict();

	curs_set(0);
	wattrset(defwin, A_NORMAL);
	werase(defwin);
	waddstr(defwin, _("Proszê czekaæ, trwa ³adowanie s³ownika..."));

	updateall();

	switch (new_dict) {
		case 0:
			idx = DEFINDEX_AP;
			dat = DEFDICT_AP;
			break;
		case 1:
			idx = DEFINDEX_PA;
			dat = DEFDICT_PA;
			break;
		case 2:
			idx = DEFINDEX_DP;
			dat = DEFDICT_DP;
			break;
		case 3:
			idx = DEFINDEX_PD;
			dat = DEFDICT_PD;
			break;
		default:
			idx = "";
			dat = "";
	}

	if (!opendict(filespath, idx, dat)) {
		switch (ydperror) {
			case YDP_CANTOPENIDX:
				showerror(_("Nie mo¿na otworzyæ pliku indeksowego."));
			case YDP_CANTOPENDEF:
				showerror(_("Nie mo¿na otworzyæ pliku z definicjami."));
			case YDP_INVALIDFILE:
				showerror(_("B³±d podczas czytania plików."));
		}
	}

	dict = new_dict;
	defline = 0;
	defupd = 1;
	menu = 0;
	menux = 0;
	pos = 0;
	if (strlen(input)) {
		menux = strlen(input);
		findword2();
	}

	curs_set(1);
	updateall();
}
