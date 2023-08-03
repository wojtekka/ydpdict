/*
 *  ydpdict
 *  (C) Copyright 1998-2023 Wojtek Kaniewski <wojtekka@toxygen.net>
 *                          Piotr Domagalski <szalik@szalik.net>
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

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_NCURSESW_NCURSES_H
#  include <ncursesw/ncurses.h>
#else
#  ifdef HAVE_CURSES_H
#    include <curses.h>
#  endif
#endif
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ydpdict/ydpdict.h>
#include <wchar.h>
#include <wctype.h>
#include <errno.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif
#define N_(x) x

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_LIBAO
#include <ao/ao.h>
#endif

#include "ydpconfig.h"
#include "ydpsound.h"
#include "xmalloc.h"

#if defined(YDPDICT_VERSION) && defined(YDPDICT_MAKE_VERSION)
#if YDPDICT_VERSION >= YDPDICT_MAKE_VERSION(1,0,3)
#define YDPDICT_HAS_SUPERSCRIPT
#endif
#endif

#ifndef COLOR_DEFAULT
#define COLOR_DEFAULT (-1)
#endif

ydpdict_t *dict;	///< Dictionary handle
int word_count;		///< Dictionary word count

wchar_t input[INPUT_LEN + 1];	///< Input word
int input_index;		///< Input word cursor index
int input_exact = 1;		///< Input word Exact match flag

int list_index;		///< Word list scroll line index
int list_page;		///< Word list scroll page
int focus;		///< Current focus (0 - word list, 1 - word definition, 2 - help screen)
int saved_focus;	///< focus stored while showing help etc.

char *def;		///< Current definition
int def_encoding;	///< Definition encoding
int def_index;		///< Definition scroll line index
int def_saved_index;	///< def_index stored while showing help etc.
int def_height;		///< Definition height in lines
int def_update;		///< Definition update flag
int def_raw_rtf;	///< Display raw RTF flag

int screen_width;	///< Screen width
int screen_height;	///< Screen height

int resized_term;	///< Terminal resize flag

WINDOW *window_word;	///< Wordlist window
WINDOW *window_def;	///< Definition window
WINDOW *window_sep;	///< Separator window
WINDOW *window_arrows;	///< Arrows window

#define ATTR_HIDE 0x10000	///< Private attribute to indicate hidden text
#define ATTR_SUPER 0x20000	///< Private attribute to indicate superscript
#define ATTR_MASK 0x30000

int color_text;
int color_cf1;
int color_cf2;

const char *qualifiers_header = N_("{\\b Grammatical qualifiers}{\\par\\pard}");

const char *qualifiers[][2] =
{
	{ "abbr", N_("abbreviation") },
	{ "acc", N_("accusative") },
	{ "adj", N_("adjective") },
	{ "adv", N_("adverb") },
	{ "art", N_("article") },
	{ "attr", N_("attribute") },
	{ "aux", N_("auxiliary") },
	{ "comp", N_("comparative degree") },
	{ "compl.", N_("complement") },
	{ "conj", N_("conjunction") },
	{ "cpd", N_("compound") },
	{ "dat", N_("dative") },
	{ "def", N_("definite") },
	{ "decl", N_("declined") },
	{ "dimin", N_("diminutive") },
	{ "excl", N_("exclamation") },
	{ "f", N_("feminine") },
	{ "fig", N_("figurative") },
	{ "fml", N_("formal") },
	{ "fus", N_("fused") },
	{ "gen", N_("genitive") },
	{ "imp", N_("imperfective") },
	{ "imperf", N_("imperfective") },
	{ "impers", N_("impersonal") },
	{ "indef", N_("indefinite") },
	{ "inf", N_("informal") },
	{ "infin", N_("infinitive") },
	{ "instr", N_("instrumental") },
	{ "inv", N_("invariable") },
	{ "irreg", N_("irregular") },
	{ "ksi", N_("literary") },
	{ "liter.", N_("literary") },
	{ "loc", N_("locative") },
	{ "m", N_("masculine") },
	{ "modal", N_("modal") },
	{ "n", N_("noun") },
	{ "nom", N_("nominative") },
	{ "npl", N_("noun plural") },
	{ "nt", N_("neuter") },
	{ "num", N_("numeral") },
	{ "nvir", N_("non-virile gender") },
	{ "old", N_("old") },
	{ "part", N_("particle") },
	{ "pej", N_("pejorative") },
	{ "perf", N_("perfective") },
	{ "pl", N_("plural") },
	{ "polite", N_("polite") },
	{ "poss.", N_("possesive") },
	{ "post", N_("postposition") },
	{ "pot", N_("informal") },
	{ "pp", N_("past participle") },
	{ "pref", N_("prefix") },
	{ "pred", N_("predicate") },
	{ "prep", N_("preposition") },
	{ "pron", N_("pronoun") },
	{ "pt", N_("past simple") },
	{ "sg", N_("singular") },
	{ "suff", N_("suffix") },
	{ "superl", N_("superlative degree") },
	{ "vb", N_("verb") },
	{ "vi", N_("intransitive verb") },
	{ "vir", N_("virile gender") },
	{ "voc", N_("vocative") },
	{ "vr", N_("reflexive verb") },
	{ "vt", N_("transitive verb") },
};

const char *qualifiers_format = "{\\line{\\cf2 %s} {%s}}";

const char *help[] =
{
	"{\\b " HEADER_NAME "}",
	"{\\line{\\b " HEADER_COPYRIGHT "}}{\\pard\\pard}",
	N_("{\\cf2 F1} or {\\cf2 ?} - this help"),
	N_("{\\line{\\cf2 Tab} - change focus}"),
	N_("{\\line{\\cf2 Up} and {\\cf2 Down} - scroll up and down}"),
	N_("{\\line{\\cf2 Page Up} and {\\cf2 Page Down} - scroll page up and down}"),
	N_("{\\line{\\cf2 F2} or {\\cf2 `} - play pronunciation sample}"),
	N_("{\\line{\\cf2 F3} or {\\cf2 <} - English-Polish dictionary}"),
	N_("{\\line{\\cf2 F4} or {\\cf2 >} - Polish-English dictionary}"),
	N_("{\\line{\\cf2 F5} or {\\cf2 [} - German-Polish dictionary}"),
	N_("{\\line{\\cf2 F6} or {\\cf2 ]} - Polish-German dictionary}"),
	N_("{\\line{\\cf2 F7} or {\\cf2 !} - grammatical qualifiers}"),
	N_("{\\line{\\cf2 Esc} - go back}"),
	N_("{\\line{\\cf2 Ctrl-U} or {\\cf2 Ctrl-W} - clear input field}"),
	N_("{\\line{\\cf2 Ctrl-L} - refresh display}"),
	N_("{\\line{\\cf2 Ctrl-C} or {\\cf2 Ctrl-X} - quit program}"),
};

const char *help_footer = N_("{\\par\\pard}Contact: {\\b %s}. Current version is always available at {\\b %s}");

/**
 * \brief Exits program
 *
 * \param msg Error message or NULL if there is no error condition
 *
 * Cleans up a bit.
 */
void show_error(const char *msg)
{
	if (window_word)
		delwin(window_word);

	if (window_sep)
		delwin(window_sep);

	if (window_def)
		delwin(window_def);

	if (window_arrows)
		delwin(window_arrows);

	werase(stdscr);
	wrefresh(stdscr);

	endwin();

	if (dict) {
		ydpdict_close(dict);
		dict = NULL;
	}

	xfree(def);

	if (msg)
		fprintf(stderr, "%s\n\n", msg);

#ifdef HAVE_LIBAO
	ao_shutdown();
#endif

	exit(msg ? 1 : 0);
}

/**
 * \brief SIGTERM handler.
 */
static void sigterm(int sig)
{
	show_error(NULL);
}

/**
 * \brief Updates and redraws all ncurses windows
 */
void update_all(void)
{
	wnoutrefresh(window_sep);
	wnoutrefresh(window_def);
	wnoutrefresh(window_arrows);
	wnoutrefresh(window_word);
	doupdate();
}

/**
 * \brief Creates and configures ncurses windows
 *
 * Configuration also means filling the header and separator window
 */
void create_windows(void)
{
	int i;

	/* Destroy windows */
	if (window_word)
		delwin(window_word);
	if (window_def)
		delwin(window_def);
	if (window_sep)
		delwin(window_sep);
	if (window_arrows)
		delwin(window_arrows);
	
	/* Create windows */
	window_word = newwin(screen_height - 2, 20, 1, 2);
	window_def = newwin(screen_height - 2, screen_width - 29, 1, 27);
	window_sep = newwin(screen_height, 4, 0, 23);
	window_arrows = newwin(screen_height, 1, 0, screen_width - 1);

	if (!window_word || !window_def || !window_sep)
		show_error(_("Out of memory"));
	
	/* Configure windows */
	keypad(window_word, TRUE);
	halfdelay(100);
	erase();

	/* Draw vertical bar */
	for (i = 0; i < screen_height; i++)
		mvwaddch(window_sep, i, 1, ACS_VLINE);
}

/**
 * \brief SIGWINCH handler
 */
void sigwinch(int sig)
{
	resized_term = 1;
}

/**
 * \brief Checks and adjusts word list selection to match the new window size
 */
void check_size(void)
{
	int newx, newy, fake = 0, diff;

	newx = stdscr->_maxx + 1;
	newy = stdscr->_maxy + 1;

	/* Minimum size */
	if (newx < 29)
		fake = newx = 29;
	if (newy < 4)
		fake = newy = 4;
	
	if (fake)
		resizeterm(newy, newx);

	screen_width = newx;
	screen_height = newy;

	diff = list_page + (screen_height - 3) - word_count;
	if (diff > 0) {
		list_page -= diff;
		list_index += diff;
	}
	diff = list_index - (screen_height - 4);
	if (diff > 0) {
		list_page += diff;
		list_index -= diff;
	}
}

/**
 * \brief Handles terminal resize
 */
void resize(void)
{
	endwin();
	refresh();

	check_size();
	create_windows();

	resized_term = 0;
}

#define is_visible(x) ((ypos >= first && ypos < (screen_height - 2) + first) ? x : "")

/**
 * \brief Prints the definition
 *
 * \param def Definition RTF source
 * \param first First line visible on the screen
 *
 * \return Total number of lines
 */
int def_print(char *def, int first)
{
	int attr = color_text, attrs[16], level = 0, lastsp = 1, xpos = 0;
	int phon = 0, lp = 0, dispword, newline_, newattr, lastnl = 0;
	int ypos = 0, margin = 0, tp, newphon;
	char token[64], line[80];

	if (!def)
		return 1;

	werase(window_def);

	if (def_raw_rtf) {
		wattrset(window_def, A_NORMAL);
		waddstr(window_def, def);
		return 1;
	}

	while (def && *def) {
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
					waddstr(window_def, is_visible("   "));
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
					newattr |= ATTR_HIDE;
				if (!strcmp(token, "super")) {
#ifndef YDPDICT_HAS_SUPERSCRIPT
					line[lp++] = '^';
					line[lp] = 0;
#endif
					newattr |= ATTR_SUPER;
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
				if (attr & ATTR_HIDE)
					break;

#ifdef YDPDICT_HAS_SUPERSCRIPT
				if ((attr & ATTR_SUPER) && isdigit(*def))
					*def -= '0';
#endif

				wattrset(window_def, attr & ~ATTR_MASK);
				lastnl = 0;
				
				switch (*def) {
					case ' ':
						if (lastsp)
							break;
						dispword = 1;
						lastsp = 1;
						if (!lp) {
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
				waddstr(window_def, is_visible("\n"));
				ypos++;
				if (margin)
					waddstr(window_def, is_visible("   "));
				xpos = ((margin) ? 3 : 0) + strlen(line);
			} else {
				xpos += strlen(line);
			}

			if (def_encoding == YDPDICT_ENCODING_UTF8) {
				waddstr(window_def, is_visible(line));
			} else {
				if (phon) {
					char *tmp = ydpdict_phonetic_to_utf8(is_visible(line));
					waddstr(window_def, tmp);
					xfree(tmp);
				} else {
#ifndef YDPDICT_HAS_SUPERSCRIPT
					char *tmp = ydpdict_windows1250_to_utf8(is_visible(line));
#else
					char *tmp = ydpdict_windows1250_super_to_utf8(is_visible(line));
#endif
					waddstr(window_def, tmp);
					xfree(tmp);
				}
			}

			if (lastsp && xpos != 50) {
				waddstr(window_def, is_visible(" "));
				xpos++;
			}

			lp = 0;
		}
		
		if (newline_ && !(attr & ATTR_HIDE)) {
			waddstr(window_def, is_visible("\n"));
			ypos++;
			xpos = (margin) ? 3 : 0;

			if (margin)
				waddstr(window_def, is_visible("   "));

			lastsp = 1;
			lastnl = 1;
		}

		attr = newattr;
		phon = newphon;
	}

	if (lp) {
		if (50 - xpos < lp) {
			waddstr(window_def, is_visible("\n"));
			ypos++;
		}

		if (def_encoding == YDPDICT_ENCODING_UTF8) {
			waddstr(window_def, is_visible(line));
		} else {
			if (phon) {
				char *tmp = ydpdict_phonetic_to_utf8(is_visible(line));
				waddstr(window_def, tmp);
				xfree(tmp);
			} else {
				char *tmp = ydpdict_windows1250_to_utf8(is_visible(line));
				waddstr(window_def, tmp);
				xfree(tmp);
			}
		}
	}

	ypos++;

	return ypos;
}

/**
 * \brief Looks up current input word in the dictionary
 */
void input_find(void)
{
	char dest[64];
	wchar_t src_buf[64];
	const wchar_t *src;
	int idx;
	int i;

	if (!dict)
		return;
	
	input_exact = 1;

	for (i = wcslen(input); i > 0; i--) {
		wcsncpy(src_buf, input, i);
		src_buf[i] = 0;
		src = src_buf;

		wcsrtombs(dest, &src, sizeof(dest), NULL);

		idx = ydpdict_find_word(dict, dest);
	
		if (idx != -1) {
			list_page = idx;
			list_index = 0;

			if (list_page > word_count - (screen_height - 3)) {
				list_page = word_count - (screen_height - 3);
				list_index = idx - list_page;
			}

			break;
		}

		input_exact = 0;
	}
}

/**
 * \brief Redraws word list
 */
void list_redraw(void)
{
	int y;

	werase(window_word);

	if (dict) {
		for (y = 0; y < (screen_height - 3); y++) {
			wattrset(window_word, (y == list_index) ? A_REVERSE : A_NORMAL);
			mvwaddstr(window_word, y + 1, 0, "                    ");
	
			if (list_page + y >= word_count)
				continue;
	
			mvwaddstr(window_word, y + 1, 1, ydpdict_get_word(dict, list_page + y));
		}
	}
	
	wattrset(window_word, input_exact ? A_BOLD : A_NORMAL);
	mvwaddstr(window_word, 0, 0, "[__________________]");

	mvwaddwstr(window_word, 0, 1, input);
	wattrset(window_word, A_NORMAL);

	wmove(window_word, 0, input_index + 1);
}

/**
 * \brief Redraws the definition
 */
void def_redraw(void)
{
	if (def_update && dict) {
		xfree(def);
		def = (char*) ydpdict_read_rtf(dict, list_page + list_index);
		def_encoding = YDPDICT_ENCODING_WINDOWS1250;
		def_update = 0;
	}
	
	list_redraw();

	def_height = def_print(def, def_index);

	curs_set((focus) ? 0 : 1);

	wattrset(window_sep, A_BOLD);
	if (focus) {
		mvwaddch(window_sep, screen_height / 2, 0, ACS_HLINE);
		mvwaddch(window_sep, screen_height / 2, 1, ACS_HLINE);
		mvwaddch(window_sep, screen_height / 2, 2, ACS_RARROW);
	} else {
		mvwaddch(window_sep, screen_height / 2, 0, ACS_LARROW);
		mvwaddch(window_sep, screen_height / 2, 1, ACS_HLINE);
		mvwaddch(window_sep, screen_height / 2, 2, ACS_HLINE);
	}

	wattrset(window_arrows, A_DIM);
	mvwaddch(window_arrows, 1, 0, (def_index > 0) ? ACS_UARROW : ' ');
	mvwaddch(window_arrows, screen_height - 2, 0, (def_index < def_height - (screen_height - 2)) ? ACS_DARROW : ' ');

	update_all();
}

void switch_dict(int new_dict)
{
	const char *idx[4] = { DEFAULT_IDX_AP, DEFAULT_IDX_PA, DEFAULT_IDX_DP, DEFAULT_IDX_PD };
	const char *dat[4] = { DEFAULT_DAT_AP, DEFAULT_DAT_PA, DEFAULT_DAT_DP, DEFAULT_DAT_PD };
	char *full_idx = NULL, *full_dat = NULL;
	struct dirent *de;
	DIR *dh;

	if (new_dict < 0 || new_dict > 3)
		return;

	if (dict) {
		ydpdict_close(dict);
		dict = NULL;
	}

	curs_set(0);
	wattrset(window_def, A_NORMAL);
	werase(window_def);
	waddstr(window_def, _("Please wait, loading the dictionary..."));

	update_all();

	errno = ENOENT;
	
	if (config_path && (dh = opendir(config_path))) {
		while ((de = readdir(dh))) {
			if (strcasecmp(de->d_name, idx[new_dict]) == 0 && full_idx == NULL) {
				full_idx = xmalloc(strlen(config_path) + 1 + strlen(de->d_name) + 1);
				sprintf(full_idx, "%s/%s", config_path, de->d_name);
			}
			if (strcasecmp(de->d_name, dat[new_dict]) == 0 && full_dat == NULL) {
				full_dat = xmalloc(strlen(config_path) + 1 + strlen(de->d_name) + 1);
				sprintf(full_dat, "%s/%s", config_path, de->d_name);
			}
		}

		closedir(dh);
	}

	if (full_idx == NULL || full_dat == NULL || !(dict = ydpdict_open(full_dat, full_idx, YDPDICT_ENCODING_UTF8))) {
		const char *tmp, *err;
		
		/* errno is set to ENOENT above for missing dictionary files */
		if (!errno)
			err = _("Invalid file format");
		else
			err = strerror(errno);

		tmp = _("{\\cf2 Error!}\\par\\pard{Unable to open dictionary: %s. Press {\\cf2 F1} or {\\cf2 ?} for help.}");

		xfree(def);
		def = xmalloc(strlen(tmp) + strlen(err));
		sprintf(def, tmp, err);
		def_encoding = YDPDICT_ENCODING_UTF8;
		def_index = 0;
		word_count = 0;
	} else {
		word_count = ydpdict_get_count(dict);
		def_update = 1;
	}

	xfree(full_idx);
	xfree(full_dat);

	config_dict = new_dict;
	def_index = 0;
	input_index = 0;

	list_index = 0;
	list_page = 0;

	if (wcslen(input)) {
		input_index = wcslen(input);
		input_find();
	}

	curs_set(1);
	update_all();
}

int main(int argc, char **argv)
{
	MEVENT m_event;
	int bg;

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif
#ifdef ENABLE_NLS
	textdomain("ydpdict");
#endif

#ifdef HAVE_LIBAO
	ao_initialize();
#endif

#ifdef SIGWINCH
	signal(SIGWINCH, sigwinch);
#endif
	signal(SIGTERM, sigterm);
	signal(SIGINT, sigterm);
	signal(SIGHUP, sigterm);

	read_config(argc, argv);

	initscr();
	noecho();
	cbreak();

	if (config_transparent) {
		bg = COLOR_DEFAULT;
		use_default_colors();
	} else
		bg = COLOR_BLACK;

	/* Prepare colors */
	if (config_color && has_colors()) {
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

	/* Prepare UI */
	check_size();
	create_windows();

	/* Load the dictionary */
	switch_dict(config_dict);

	mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);

	if (config_word) {
		const char *src = config_word;

		mbsrtowcs(input, &src, INPUT_LEN, NULL);
		input_find();
		input_index = wcslen(input);
	}
	
	for (;;) {
		int ch;

		if (resized_term)
			resize();

		def_redraw();

		ch = wgetch(window_word);
		
		switch (ch) {

#define __MOUSE_IN(window, event, correct1, correct2, correct3, correct4) (\
	 event.y > (window->_begy + correct1) && event.y < (window->_begy + window->_maxy + correct2) &&\
	 event.x > (window->_begx + correct3) && event.x < (window->_begx + window->_maxx + correct4))

			case KEY_MOUSE:
				if (getmouse(&m_event) == OK) {

					/* Double-click points to the word */
					if (m_event.bstate & BUTTON1_DOUBLE_CLICKED && __MOUSE_IN(window_def, m_event, -2, 1, -3, 2)) {
						wchar_t ch, buf[INPUT_LEN + 1];
						cchar_t cc;
						int i = 0, x = m_event.x - 27;

						mvwin_wch(window_def, m_event.y - 2, x, &cc);
						ch = cc.chars[0];
						

						if (!(iswalpha(ch) || ch == L'-'))
							break;

						/* Look for first non-letter */
						do {
							x--;
							mvwin_wch(window_def, m_event.y - 2, x, &cc);
							ch = cc.chars[0];
						} while ((iswalpha(ch) || ch == L'-') && x >= 0);

						/* And collect characters on the right */
						do {
							x++;
							mvwin_wch(window_def, m_event.y - 2, x, &cc);
							ch = cc.chars[0];
							buf[i++] = ch;
						} while ((iswalpha(ch) || ch == L'-') && x < window_def->_maxx);

						buf[--i] = L'\0';

						/* Did we find anything? */
						if (wcslen(buf)) {
							wcsncpy(input, buf, INPUT_LEN);
							input[INPUT_LEN] = 0;
							input_index = wcslen(input);
							input_find();
							def_update = 1;
						}

						break;
					}

					if (m_event.bstate & BUTTON1_CLICKED || m_event.bstate & BUTTON1_DOUBLE_CLICKED) {
						/* Clicked on the word list */
						if (__MOUSE_IN(window_word, m_event, -2, 1, -1, 1) && m_event.y > 2) {
							list_index = m_event.y - 3;
							memset(input, 0, sizeof(input));
							input_index = 0;
							def_index = 0;
							def_update = 1;
						}

						/* Header */
						if (m_event.y == 0) {
							ungetch(KEY_F(1));
							break;
						}

						/* Scrolling */
						if (m_event.y == screen_height - 1) {
							ungetch(KEY_NPAGE);
							break;
						}

						if (m_event.y <= 1) {
							ungetch(KEY_PPAGE);
							break;
						}

					}

					if (m_event.bstate & BUTTON1_CLICKED) {
						/* Change of the active window */
						if (__MOUSE_IN(window_word, m_event, -2, 1, -3, 2) && focus) {
							if (focus == 2)
								def_update = 1;
							focus = 0;
						}

						if (__MOUSE_IN(window_def, m_event, -2, 1, -3, 2) && !focus)
							focus = 1;
					}
				}

				break;
#undef isalpha_pl_PL
#undef __MOUSE_IN

			case 10: /* Enter */
				if (focus) {
					if (def_index < def_height - (screen_height - 2))
						def_index++;
				} else {
					wchar_t *c = input + wcslen(input) - 1;

					while (wcslen(input) > 1 && *c == L' ')
						*c-- = 0;

					input_index = wcslen(input);

					input_find();
					def_update = 1;
				}
				break;

			case 27: /* ESC */
				if (focus == 2) {
					focus = saved_focus;
					def_update = 1;
					def_index = def_saved_index;
				} else if (focus == 1) {
					focus = 0;
				} else {
					memset(&input, 0, sizeof(input));
					list_index = 0;
					list_page = 0;
					input_index = 0;
					def_index = 0;
					def_update = 1;
				}
				break;

			case 24: /* Ctrl-X */
				show_error(NULL);
				break;

#ifdef KEY_RESIZE
			case KEY_RESIZE:
				resized_term = 1;
				break;
#endif

			case 9: /* TAB */
				if (focus == 2) {
					def_update = 1;
					def_index = 0;
				}
				focus = (focus) ? 0 : 1;
				break;

			case KEY_F(2):
			case '`':
				if (config_dict == 0 || config_dict == 2)
					play_sample(config_dict, list_page + list_index);

				break;

			case KEY_F(1):
			case '?':
			{
				unsigned int len = 0;
				int i;

				for (i = 0; i < sizeof(help) / sizeof(help[0]); i++)
					len += strlen(gettext(help[i]));

				len += strlen(gettext(help_footer));

				xfree(def);

				def = xmalloc(len + strlen(HELP_EMAIL) + strlen(HELP_WEBSITE) + 1);
				def[0] = 0;

				for (i = 0; i < sizeof(help) / sizeof(help[0]); i++)
					strcat(def, gettext(help[i]));

				sprintf(def + strlen(def), gettext(help_footer), HELP_EMAIL, HELP_WEBSITE);

				def_saved_index = def_index;
				def_index = 0;
				def_encoding = YDPDICT_ENCODING_UTF8;
				saved_focus = focus;
				focus = 2;

				break;
			}

			case KEY_F(3):
			case '<':
				if (config_dict != 0)
					switch_dict(0);

				break;

			case KEY_F(4):
			case '>':
				if (config_dict != 1)
					switch_dict(1);

				break;

			case KEY_F(5):
			case '[':
				if (config_dict != 2)
					switch_dict(2);

				break;

			case KEY_F(6):
			case ']':
				if (config_dict != 3)
					switch_dict(3);

				break;

			case KEY_F(7):
			case '!':
			{
				unsigned int len = 0;
				int i;

				len += strlen(gettext(qualifiers_header));

				for (i = 0; i < sizeof(qualifiers) / sizeof(qualifiers[0]); i++) {
					len += strlen(qualifiers_format);
					len += strlen(qualifiers[i][0]);
					len += strlen(gettext(qualifiers[i][1]));
				}

				xfree(def);

				def = xmalloc(len);

				strcpy(def, gettext(qualifiers_header));

				for (i = 0; i < sizeof(qualifiers) / sizeof(qualifiers[0]); i++)
					sprintf(def + strlen(def), qualifiers_format, qualifiers[i][0], gettext(qualifiers[i][1]));

				def_saved_index = def_index;
				def_index = 0;
				def_encoding = YDPDICT_ENCODING_UTF8;
				saved_focus = focus;
				focus = 2;

				break;
			}
				

			case KEY_UP:
				if (focus) {
					if (def_index > 0)
						def_index--;
				} else {
					if (list_index > 0)
						list_index--;
					else
						if (list_page > 0)
							list_page--;
					def_index = 0;
					def_update = 1;
				}
				break;

			case KEY_PPAGE:
				if (focus) {
					if (def_index > screen_height - 3)
						def_index -= screen_height - 2;
					else
						def_index = 0;
				} else {
					if (list_index > 0)
						list_index = 0;
					else if (list_page > screen_height - 4)
						list_page -= screen_height - 3;
					else
						list_page = 0;

					def_index = 0;
					def_update = 1;
				}
				break;

			case KEY_DOWN:
				if (focus) {
					if (def_index < def_height - (screen_height - 2))
						def_index++;
				} else {
					if (list_index < screen_height - 4 && (list_index + 1 < word_count))
						list_index++;
					else if (list_page < word_count - (screen_height - 3))
						list_page++;
					def_index = 0;
					def_update = 1;
				}
				break;

			case KEY_NPAGE:
				if (focus) {
					if (def_index < def_height - (screen_height - 2) * 2 - 1)
						def_index += screen_height - 2;
					else
						def_index = def_height - (screen_height - 2);
				} else {
					if (list_index < screen_height - 4)
						list_index = screen_height - 4;
					else if (list_page < word_count - (screen_height - 3) * 2 - 1)
						list_page += screen_height - 3;
					else
						list_page = word_count - (screen_height - 3);
					def_index = 0;
					def_update = 1;
				}
				break;

			case KEY_F(9):
				def_raw_rtf = !def_raw_rtf;
				break;

			case KEY_DC:
				if (!focus) {
					if (input_index < wcslen(input)) {
						memmove(input + input_index, input + input_index + 1, (wcslen(input) - input_index + 1) * sizeof(wchar_t));
						input_find();
						def_update = 1;
					} else
						beep();
				}
				break;

			case KEY_BACKSPACE:
			case 127:
			case 8:
				if (focus) {
					if (def_index > 0)
						def_index--;
					break;
				} else {
					if (input_index) {
						memmove(input + input_index - 1, input + input_index, (wcslen(input) - input_index + 1) * sizeof(wchar_t));
						input_index--;
						input_find();
						def_update = 1;
					} else
						beep();
				}
				break;

			case 12: /* Ctrl-L */
				resized_term = 1;
				break;
				
			case 21: /* Ctrl-U */
			case 23: /* Ctrl-W */
				memset(&input, 0, sizeof(input));
				list_index = 0;
				list_page = 0;
				input_index = 0;
				def_index = 0;
				def_update = 1;
				focus = 0;
				break;

			case KEY_HOME:
			case KEY_FIND:
				input_index = 0;
				break;

			case KEY_END:
			case KEY_SELECT:
				input_index = wcslen(input);
				break;

			case KEY_LEFT:
				if (!input_index || focus)
					beep();
				else
					input_index--;
				break;

 			case KEY_RIGHT:
				if (input_index >= wcslen(input) || focus)
					beep();
				else
					input_index++;
				break;

			default:
			{
				static char mb[5];
				static int mblen = 0, mbofs = 0;
				int len;
				wchar_t wc;
	
				if (ch < ' ' || ch > 255)
					break;

				if ((ch & 0xe0) == 0xc0) {
					mb[0] = ch;
					mblen = 2;
					mbofs = 1;
					break;
				}

				if ((ch & 0xf0) == 0xe0) {
					mb[0] = ch;
					mblen = 3;
					mbofs = 1;
					break;
				}

				if ((ch & 0xf8) == 0xf0) {
					mb[0] = ch;
					mblen = 4;
					mbofs = 1;
					break;
				}

				if ((ch & 0xc0) == 0x80) {
					if (!mblen)
						break;

					mb[mbofs++] = ch;

					if (mbofs == mblen) {
						mb[mbofs] = 0;

						mbrtowc(&wc, mb, strlen(mb), NULL);
					} else
						break;
				} else {
					wc = ch;
				}

				if (focus) {
					memset(input, 0, sizeof(input));
					input_index = 0;
				}

				if (wcslen(input) > INPUT_LEN)
					break;

				if (input_index < wcslen(input))
					memmove(input + input_index + 1, input + input_index, (wcslen(input) - input_index + 1) * sizeof(wchar_t));
				
				input[input_index++] = wc;

				len = wcslen(input);

				if (len >= 2 && !wcscmp(input + len - 2, L":q"))
					show_error(NULL);

				if (len >= 3 && !wcscmp(input + len - 3, L":wq"))
					show_error(NULL);

				if (len >= 2 && !wcscmp(input + len - 2, L":x"))
					show_error(NULL);

				input_find();
				def_update = 1;
				focus = 0;
				def_index = 0;
				break;
			}
		}
	}
	
	show_error(NULL);

	return 0;
}


