ydpdict
=======

Program, którego dokumentację w tej chwili czytasz, służy do obsługi
multimedialnych słowników angielsko-polskiego i polsko-angielskiego Collinsa
oraz niemiecko-polskiego i polsko-niemieckiego Langenscheidta. Oryginalne
pakiety oferowane przez firmę [Young Digital Planet](http://www.ydp.com.pl)
zawierają jedynie programy dla systemu Windows, co uniemożliwia
korzystanie z nich pod innymi systemami. Niniejszy program pozwala korzystać
z tych słowników pod kontrolą Linuksa. Możliwe, że zadziała również pod
innymi systemami, ponieważ jest pisany w miarę możliwości przenośnie.

![Zrzut ekranu](doc/ydpdict.png)

Do prawidłowej pracy niezbędna jest dystrybucja Linuksa (lub innego systemu
uniksowego) z biblioteką ncurses wspierającą Unikod oraz zainstalowana
biblioteka [libydpdict](https://github.com/wojtekka/libydpdict). W przypadku
systemu operacyjnego, który nie wspiera Unikodu, należy użyć wersji
wcześniejszej niż 0.99. Aby skompilować i zainstalować oficjalne wydanie,
należy po rozpakowaniu wykonać polecenia (ostatnie z nich z uprawnieniami
administratora, np. przez `sudo`):

    ./configure
    make
    make install

Przy kompilacji źródeł pobranych z repozytorium należy wcześniej wykonać
jeszcze polecenie:

    ./autogen.sh

Ze względu na domyślne zachowanie autoconfa, strony manuali będą instalowane
w katalogu `$(prefix)/man`, zamiast `$(prefix)/share/man`. Jeśli chcesz je
zainstalować w odpowiednim miejscu, wpisz:

    ./configure --mandir=/usr/local/share/man
    make
    make install

Następnie należy skopiować cztery pliki z katalogu `database` oryginalnego,
pracującego pod systemem Windows programu:

* dict100.dat
* dict100.idx
* dict101.dat
* dict101.idx

dla słownika angielsko-polskiego i polsko-angielskiego, lub:

* dict200.dat
* dict200.idx
* dict201.dat
* dict201.idx

dla słownika niemiecko-polskiego i polsko-niemieckiego, do katalogu,
w którym znajduje się program. Dodatkowo program będzie szukał plików
słownika w systemowym katalogu, np. `/usr/local/share/ydpdict` jeśli do
skryptu `./configure` nie przekazano parametru `--prefix` lub `/usr/share/ydpdict`
jeśli program pochodzi z pakietu dystrybucji). Można też podać ścieżkę,
w której się one znajdują w parametrze `Path` pliku konfiguracyjnego.
Należy również zmienić parametr `CDPath` na ścieżkę, gdzie znajduje się
podmontowana płyta CD. Jego brak uniemożliwi odtwarzanie próbek dźwiękowych.
Aby móc odtwarzać próbki ze słownika angielsko-polskiego
i niemiecko-polskiego, należy utworzyć katalog, a w nim katalog `en`
z zawartością płyty CD słownika angielsko-polskiego oraz katalog `de`
z plikami słownika niemiecko-polskiego.  

Obsługa programu jest bajecznie prosta. Wystarczy wpisać szukane słowo
lub wybrać je klawiszami kursorów. Do przemieszania się między listą
słów, a definicją służy klawisz Tab. Odtwarzanie próbek to F2. Zmiana na
inny słownik to F3, F4, F5, F6.

Program obsługiwać można częściowo za pomocą myszki. Kliknięcie na słowo
z listy, spowoduje wyświetlenie jego tłumaczenia. Klikając na dolne i górne
krawędzie ekranu, można przewijać listę lub opis. Jeśli klikniemy dwukrotnie
na słowo z okienka tłumaczenia, zostanie ono automatycznie wyszukane.

Licencja
--------

(C) Copyright 1998-2023 Wojtek Kaniewski <wojtekka@toxygen.net>

ydpdict jest wydany na licencji GPL w wersji 2 (treść w języku angielskim
dostępna w pliku COPYING). Wszelkie poprawki wysłane do autora na licencji
innej niż GPL w wersji 2 zostaną odrzucone. Brak informacji o licencji
będzie traktowany jak przyjęcie licencji kodu, który jest poprawiany.
Wyjątkiem są pliki adpcm.c i adpcm.h autorstwa Stanley'a J. Brooksa wydane
na licencji LGPL w wersji 2 i dostosowane do współczesnych kompilatorów:

Copyright (C) 1999 Stanley J. Brooks <stabro@megsinet.net>

Lista zmian
-----------

  - 1.0.5 (2023-12-22): Kolejne poprawki użycia gettext. Poprawka sięgania
    do wnętrza struktur ncurses Svena Joachima.
  - 1.0.4 (2023-10-21): Usunięcie nagłówka z wersją i copyrightami. Szerokość
    definicji będzie wypełniać szerokość terminala. Poprawka kompilacji
    z gettext. Szukanie pliku konfiguracyjnego najpierw zgodnie ze
    specyfikacją XDG Base Directory ($XDG_CONFIG_HOME/ydpdict.conf lub
    $HOME/.config/ydpdict.conf), następnie tak jak poprzednie wersje
    w ~/ydpdict.conf, ~/.ydpdictrc oraz /etc/ydpdict.conf. Nowe wersje
    automake, autoconf, gettext, libtool.
  - 1.0.3 (2020-08-09): Uzupełnienie dokumentacji o domyślne katalogi z plikami
    słownika zgodnie z sugestiami Adriana Mroczkowskiego (athek@o2.pl).
    Poprawka budowania z nowymi wersjami GCC po zgłoszeniu problemu przez
    Marcina Owsianego.
  - 1.0.2 (2013-01-11): Poprawka błędnego wyświetlania słów z końca słownika
    zgłoszone przez Jakuba Wilka.
  - 1.0.1 (2011-05-10): Rozszerzenie pomocy o oznaczenia kwalifikatorów
    gramatycznych oraz zmiana sposobu działania klawisza Esc: obie modyfikacje
    autorstwa Cezarego M. Kruka (c.kruk@bigfoot.com). Poprawki dokumentacji.
    Poprawka roku copyrightów. Poprawka odtwarzania niektórych próbek
    dźwiękowych Pawła Sobczaka (psobcza@o2.pl). Dodanie obsługi próbek
    dźwiękowych słownika angielskiego i niemieckiego wg pomysłu Mateusza B.
    (incusiek@gmail.com). Poprawka błędu odtwarzania niemieckich próbek
    zgłoszonego przez Krzysztofa Sucheckiego (hetteh@life.pl). Poprawka
    kompilacji bez libao. Lepsze wyświetlanie indeksów górnych (¹, ²...
    zamiast ^1, ^2...), jeśli libydpdict jest w wersji 1.0.3 lub nowszej.
    Poprawka błędu współpracy z nowymi wersjami libao zgłoszonego przez Pawła
    Sobczaka.
  - 1.0.0 (2008-02-26): Poprawka błędu zawijania zgłoszonego przez
    Marcina Owsianego. Aktualizacja pliku z tłumaczeniem.
  - 0.99.3 (2007-12-16): Poprawki parametrów wywołania programu Tomasza
    Kojma (tkojm@clamav.net). Dostosowanie do API libydpdict-0.99.3.
  - 0.99.2 (2007-08-19): Uniezależnienie od wielkości liter w nazwach
    plików słownika na podstawie poprawki Marcina Owsianego
    (marcin@owsiany.pl).
  - 0.99.1 (2007-08-09): Poprawka błędu kompilacji bez libao zgłoszonego
    przez Piotra Jaroszyńskiego (peper@gentoo.org). Poprawki dokumentacji,
    dodanie parametru skryptu configure określającego domyślny katalog
    słownika (marcin@owsiany.pl).
  - 0.99.0 (2007-02-26): Całkowita zmiana architektury -- wydzielenie
    biblioteki obsługującej słowniki, modyfikacja kodu do poprawnej pracy
    w Unikodzie, dodanie obsługi próbek dźwiękowych w formacie ADPCM.

  Lista zmian starej gałęzi kodu:
  - 0.67 (2006-xx-xx): Dodanie $(DESTDIR) przy instalacji przez Stanisława
                       Halika (weirdo%tehran.lain.pl).
  - 0.66 (2006-01-02): Poprawka odtwarzania próbek Tomasza Jakuba Skrynnyka
                       (skrynnyk%post.pl). Poprawka obsługi parametrów Jasia
		       (jasiu%tool.eu.org). Poprawka skryptów budowania.
  - 0.65 (2005-11-16): Poprawki Jakuba Wilka (jwilk%jwilk.net): usunięcie
                       złych zachowań prowadzących do segfaultów. Dodanie
		       informacji o licencji poprawek. Kosmetyka dokumentacji.
  - 0.64 (2005-10-18): Poprawki strony manuala Kamila Jarosławskiego
                       (kamil%visual.pl). Obsługa próbek o rozszerzeniu .ogg
		       Tomasza Marciniaka (ciniol%atos.wmid.amu.edu.pl).
		       Oficjalne włączenie poprawki Michała Maternika
		       (michal_m%ks.k.pl) pozwalającej korzystać z
		       niemieckiego słownika. Poprawka działania na amd64
		       Arkadiusza Kołacza (akolacz%gmail.com).
  - 0.63 (2004-05-31): Poprawka adresu. Poprawki Marcina Owsianego
                       (marcin%owsiany.pl): poprawne zamykanie plików
		       słownika, usunięcie czytania konfiguracji z aktualnego
		       katalogu, napisanie stron manuali.
  - 0.62 (2003-11-23): Poprawka Piotra Domagalskiego (szalik%szalik.net):
                       przezroczyste tło w atermie. Poprawki Jarosława
		       Kruka (jareq%pld-linux.org): skok do danego słowa
		       z linii komend, zmiana adresów PLD na aktualne.
  - 0.61 (2003-10-01): Wyświetlanie pomocy zaraz po uruchomieniu programu.
                       poprawki Cezarego M. Kruka: substytuty dla klawiszy
		       funkcyjnych, blokada odtwarzania próbek dźwiękowych
		       podczas przeglądania słownika polsko-angielskiego,
		       szczegółowa pomoc, drobne zmiany w transkrypcji
		       fonetycznej, korekta zapobiegająca śmieceniu po ekranie
		       podczas odtwarzania plików w formacie MP3.
		       Dodane skrypty do konwersji z formatu WAV do MP3.
  - 0.60 (2003-08-27): Poprawki Piotra Domagalskiego: buforowanie odczytu
                       przyspieszające start programu.
  - 0.59 (2003-07-22): Poprawki Jerzego Kędra (jxa%adm.onet.pl): program
                       nie zajmuje już 100% czasu procesora. Patch Michała
		       Maternika (michal_m%ks.k.pl) pozwalający korzystać
		       z niemieckiego słownika w pliku Deutsch.diff.
  - 0.58 (2003-05-16): Poprawki Piotra Domagalskiego: prosta obsługa
                       myszki, możliwość edycji wpisywanego słowa, Enter
                       zatwierdza i wyszukuje, lepsza reakcja na zmianę
                       rozmiaru terminala, CTRL-L, kosmetyka, xmalloc.
  - 0.57 (2003-04-15): Kolejne poprawki Piotra Domagalskiego porządkujące
                       kompilację na nielinuksowych systemach. poprawka
		       Pawła Fenglera (pawfen%wp.pl): wywoływanie
		       zewnętrznego odtwarzacza w poprawny sposób.
  - 0.56 (2003-03-21): Wiosenne porządki w kodzie; obsługa plików bazy
                       z nazwami pisanymi wielkimi literami; obsługa próbek
		       dźwiękowych o rozszerzeniu mp3; uzupełnienie
		       dokumentacji na podstawie archiwalnej wersji 0.5.
  - 0.55 (2003-03-12): Poprawki Piotra Domagalskiego (szalik%szalik.net):
                       poprawna kompilacja na OpenBSD, dokładniejsze
		       formatowanie tekstu.
  - 0.54 (2003-01-27): Poprawki Pawła Jabłońskiego
                       (pawelj%lodz.dialog.net.pl): wyświetlenie innego
		       słowa przewija okno definicji do góry; rozpoczęcie
		       pisania słowa, bedąc w oknie definicji, usuwa
		       poprzednie; klawisz F12 równoważny z Ctrl-U.
  - 0.53 (2003-01-07): Z powodu awarii dysku straciłem poprawki, które
                       podesłano mi od wydania 0.52. dlatego proszę o
		       kontakt. Poprawienie klawisza Backspace.
  - 0.52 (2002-05-27): autoconf, poprawki Grzegorza Goławskiego
                       (grzegol%pld-linux.org), poprawne działanie na
		       maszynach bigendianowych.
  - 0.51 (1999-10-17): Stabilna wersja 0.5, które nigdy nie wyszło poza
                       wersję rozwojową.
  - 0.5 (1999-08-17): Wersja rozwojowa. Spora reorganizacja kodu; pozbycie
                      się SLanga.
  - 0.4 (1998-xx-xx): Dawno temu i nieprawda.

