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

#ifndef __YDPCONVERT_H
#define __YDPCONVERT_H

#ifndef u_char
#  define u_char unsigned char
#endif

#define _(x) convert_plain(x, charset, 1)

/* UWAGA! w chwili obecnej zmienna `unicode' oznacza obecno¶æ czcionki
   z kulaw± transkrypcj± fonetyczn± */

/* konwersja uwzglêdnia jedynie polskie znaki */

#define TABLE_CP_ISO { \
  128, 129, 130, 131, 132, 133, 134, 135, \
  136, 137, 138, 139, 166, 141, 142, 172, \
  144, 145, 146, 147, 148, 149, 150, 151, \
  152, 153, 154, 155, 182, 157, 158, 188, \
  160, 161, 162, 163, 164, 161, 166, 167, \
  168, 169, 170, 171, 172, 173, 174, 175, \
  176, 177, 178, 179, 180, 181, 182, 183, \
  184, 177, 186, 187, 188, 189, 190, 191, \
  192, 193, 194, 195, 196, 197, 198, 199, \
  200, 201, 202, 203, 204, 205, 206, 207, \
  208, 209, 210, 211, 212, 213, 214, 215, \
  216, 217, 218, 219, 220, 221, 222, 223, \
  224, 225, 226, 227, 228, 229, 230, 231, \
  232, 233, 234, 235, 236, 237, 238, 239, \
  240, 241, 242, 243, 244, 245, 246, 247, \
  248, 249, 250, 251, 252, 253, 254, 255 }

/* zamienia iso na bez-iso */

#define TABLE_ISO_PLAIN { \
  '.', '.', '.', '.', '.', '.', '.', '.', \
  '.', '.', '.', '.', '.', '.', '.', '.', \
  '.', '.', '.', '.', '.', '.', '.', '.', \
  '.', '.', '.', '.', '.', '.', '.', '.', \
  ' ', 'A', '\'', 'L', '$', 'l', 'S', 'S', \
  '"', 'S', 'S', 'T', 'Z', '-', 'Z', 'Z', \
  '\'', 'a', ',', 'l', '\'', 'l', 's', '\'', \
  ',', 's', 's', 't', 'z', '\"', 'z', 'z', \
  'R', 'A', 'A', 'A', 'A', 'L', 'C', 'C', \
  'C', 'E', 'E', 'E', 'E', 'I', 'I', 'D', \
  'D', 'N', 'N', 'O', 'O', 'O', 'O', 'x', \
  'R', 'U', 'U', 'U', 'U', ' ', 'T', 'B', \
  'r', 'a', 'a', 'a', 'a', 'l', 'c', 'c', \
  'c', 'e', 'e', 'e', 'e', 'i', 'i', 'd', \
  'd', 'n', 'n', 'o', 'o', 'o', 'o', '/', \
  'r', 'u', 'u', 'u', 'u', 'y', 't', '.' }

/* pe³na konwersja do unikodu -- iso (160..255) i fonetyczne (128..159)
   (spora czê¶æ tablicy pochodzi z pakietu fonty autorstwa Qrczaka) */

#define TABLE_UNICODE { \
  ".", ".", "É”", "Ê’", ".", "Êƒ", "É›", "ÊŒ", \
  "Ç", "Î¸", "É©", "É‘", ".", "Ë", "Â´", ".", \
  "Æ", ".", ".", ".", ".", ".", ".", "Ã°", \
  "Ã¦", ".", ".", ".", ".", ".", ".", ".", \
  "Â ", "Ä„", "Ë˜", "Å", "Â¤", "Ä½", "Åš", "Â§", \
  "Â¨", "Å ", "Å", "Å¤", "Å¹", "Â­", "Å½", "Å»", \
  "Â°", "Ä…", "Ë›", "Å‚", "Â´", "Ä¾", "Å›", "Ë‡", \
  "Â¸", "Å¡", "ÅŸ", "Å¥", "Åº", "Ë", "Å¾", "Å¼", \
  "Å”", "Ã", "Ã‚", "Ä‚", "Ã„", "Ä¹", "Ä†", "Ã‡", \
  "ÄŒ", "Ã‰", "Ä˜", "Ã‹", "Äš", "Ã", "Ã", "Ä", \
  "Ä", "Åƒ", "Å‡", "Ã“", "Ã”", "Å", "Ã–", "Ã—", \
  "Å˜", "Å®", "Ãš", "Å°", "Ãœ", "Ã", "Å¢", "ÃŸ", \
  "Å•", "Ã¡", "Ã¢", "Äƒ", "Ã¤", "Äº", "Ä‡", "Ã§", \
  "Ä", "Ã©", "Ä™", "Ã«", "Ä›", "Ã­", "Ã®", "Ä", \
  "Ä‘", "Å„", "Åˆ", "Ã³", "Ã´", "Å‘", "Ã¶", "Ã·", \
  "Å™", "Å¯", "Ãº", "Å±", "Ã¼", "Ã½", "Å£", "Ë™" }

/* co tu du¿o mówiæ. zamienia fonetyczne na iso */

#define TABLE_PHONETIC_ISO { \
  ".", ".", "<c>", "<3>", ".", "<|>", "<E>", "<^>", \
  "<e>", "<0>", "<i>", "<a>", ".", ":", "´", ".", \
  "<n>", ".", ".", ".", ".", ".", ".", "<o>", \
  "<ae>", ".", ".", ".", ".", ".", ".", ".", \
  " ", "¡", "¢", "£", "¤", "¥", "¦", "§", \
  "¨", "©", "ª", "«", "¬", "­", "®", "¯", \
  "°", "±", "²", "³", "´", "µ", "¶", "·", \
  "¸", "¹", "º", "»", "¼", "½", "¾", "¿", \
  "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", \
  "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï", \
  "Ğ", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", \
  "Ø", "Ù", "Ú", "Û", "Ü", "İ", "Ş", "ß", \
  "à", "á", "â", "ã", "ä", "å", "æ", "ç", \
  "è", "é", "ê", "ë", "ì", "í", "î", "ï", \
  "ğ", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", \
  "ø", "ù", "ú", "û", "ü", "ı", "ş", "ÿ" }

u_char *convert_cp1250(u_char *inp, int alloc);
u_char *convert_plain(u_char *inp, int charset, int alloc);
u_char *convert_phonetic(u_char *inp, int charset, int alloc);

#endif /* __YDPCONVERT_H */
