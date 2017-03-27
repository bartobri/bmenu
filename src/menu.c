#include <ncurses.h>
#include <string.h>
#include "menu.h"

#define SPACE 32

void menu_init(void) {
	// Start and initialize curses mode
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();

	// Setting up and starting colors if terminal supports them
	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_CYAN, COLOR_BLACK);
		init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(3, COLOR_CYAN, COLOR_CYAN);
		attron(COLOR_PAIR(1));
	}
}

/*
 * Prints the window header (title and crossbar).
 */
void menu_header(char *version) {
	int windowRows, windowCols;
	
	(void)windowRows;
	
	getmaxyx(stdscr, windowRows, windowCols);
	
	attron(A_BOLD);
	mvprintw(0, 1, "%s%s", "B-MENU v", version);
	attroff(A_BOLD);

	move(1,0);
	hline(ACS_HLINE, windowCols);
}

/*
 * Prints the inner and outer borders for the menu.
 */
void menu_decorate(char **menu, char *title) {
	int borderCols, borderRows, startRow, startCol;
	int row, col;
	int windowRows, windowCols;
	
	getmaxyx(stdscr, windowRows, windowCols);

	// Border size (inner)
	borderCols = menu_cols(menu) + 8;
	borderRows = menu_rows(menu) + 4;

	// Minimum border width is 25 cols.
	// Need at least this much for select/exit options.
	if (borderCols < 25)
		borderCols = 25;

	// Determining starting row and column for border (inner)
	startCol = (windowCols / 2) - (borderCols / 2);
	startRow = (windowRows / 2) - (borderRows / 2);
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;

	// printing border (inner)
	for (row = 0; row < borderRows; ++row)
		for (col = 0; col < borderCols; ++col)
			if (row == 0 && col == 0)
				mvaddch(row + startRow, col + startCol, ACS_ULCORNER);
			else if (row == 0 && col == borderCols - 1)
				mvaddch(row + startRow, col + startCol, ACS_URCORNER);
			else if (row == 0)
				mvaddch(row + startRow, col + startCol, ACS_HLINE);
			else if (row == borderRows - 1 && col == 0)
				mvaddch(row + startRow, col + startCol, ACS_LLCORNER);
			else if (row == borderRows - 1 && col == borderCols - 1)
				mvaddch(row + startRow, col + startCol, ACS_LRCORNER);
			else if (row == borderRows - 1)
				mvaddch(row + startRow, col + startCol, ACS_HLINE);
			else if (col == 0)
				mvaddch(row + startRow, col + startCol, ACS_VLINE);
			else if (col == borderCols - 1)
				mvaddch(row + startRow, col + startCol, ACS_VLINE);

	// Printing (inner) border title
	attron(A_BOLD);
	mvprintw(startRow - 1, startCol, "%s", title);
	attroff(A_BOLD);

	// Border size (outer)
	borderCols += 4;
	borderRows += 4;

	// Determining starting row and column for border (outer)
	startCol = ((windowCols / 2) - (borderCols / 2));
	startRow = ((windowRows / 2) - (borderRows / 2));
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;

	// Add to the bottom of the border (outer) for select/exit options
	borderRows += 3;

	// printing border (outer)
	for (row = 0; row < borderRows; ++row)
		for (col = 0; col < borderCols; ++col)
			if (row == 0 && col == 0) {
				mvaddch(row + startRow, col + startCol, ACS_ULCORNER);
			} else if (row == 0 && col == borderCols - 1) {
				mvaddch(row + startRow, col + startCol, ACS_URCORNER);
			} else if (row == 0) {
				mvaddch(row + startRow, col + startCol, ACS_HLINE);
			} else if (row == borderRows - 1 && col == 0) {
				mvaddch(row + startRow, col + startCol, ACS_LLCORNER);
			} else if (row == borderRows - 1 && col == borderCols - 1) {
				mvaddch(row + startRow, col + startCol, ACS_LRCORNER);
				if (has_colors()) {
					attron(COLOR_PAIR(3));
					mvaddch(row + startRow + 1, col + startCol + 1, ' ');
					attron(COLOR_PAIR(1));
				}
			} else if (row == borderRows - 1) {
				mvaddch(row + startRow, col + startCol, ACS_HLINE);
				if (has_colors()) {
					attron(COLOR_PAIR(3));
					mvaddch(row + startRow + 1, col + startCol + 1, ' ');
					attron(COLOR_PAIR(1));
				}
			} else if (col == 0 && row == borderRows - 3) {
				mvaddch(row + startRow, col + startCol, ACS_LTEE);
			} else if (col == borderCols - 1 && row == borderRows - 3) {
				mvaddch(row + startRow, col + startCol, ACS_RTEE);
				if (has_colors()) {
					attron(COLOR_PAIR(3));
					mvaddch(row + startRow + 1, col + startCol + 1, ' ');
					attron(COLOR_PAIR(1));
				}
			} else if (col == 0) {
				mvaddch(row + startRow, col + startCol, ACS_VLINE);
			} else if (col == borderCols - 1) {
				mvaddch(row + startRow, col + startCol, ACS_VLINE);
				if (has_colors()) {
					attron(COLOR_PAIR(3));
					mvaddch(row + startRow + 1, col + startCol + 1, ' ');
					attron(COLOR_PAIR(1));
				}
			} else if (row == borderRows - 3) {
				mvaddch(row + startRow, col + startCol, ACS_HLINE);
			}
}

/*
 * Prints the menu options list and the select/exit
 * options. Also highlights the current selected
 * options.
 */
void menu_print(char **menu, int lo, int fo) {
	int row, startRow, startCol;
	int menuRows = menu_rows(menu);
	int menuCols = menu_cols(menu);
	int windowRows, windowCols;
	
	getmaxyx(stdscr, windowRows, windowCols);

	// Determining starting row and column for menu
	startCol = ((windowCols / 2) - (menuCols / 2)) > 0 ? ((windowCols / 2) - (menuCols / 2)) : 0;
	startRow = ((windowRows / 2) - (menuRows / 2)) > 0 ? ((windowRows / 2) - (menuRows / 2)) : 0;

	// Inserting menu in to terminal window
	for (row = 0; row < menuRows; ++row) {

		// Printing selection marker if on selected row, and removing any previous
		// marker if not.
		if (row == lo - 1) {
			if (has_colors())
				attron(COLOR_PAIR(2));
			attron(A_BOLD);
			mvaddch(row + startRow, startCol - 2, ACS_RARROW);
			mvaddstr(row + startRow, startCol, menu[row]);
			attroff(A_BOLD);
			if (has_colors())
				attron(COLOR_PAIR(1));
		} else {
			mvaddch(row + startRow, startCol - 2, SPACE);
			mvaddstr(row + startRow, startCol, menu[row]);
		}

		// printing menu foot options (select/exit)
		if (row == menuRows - 1) {
			int sRow = row + startRow + 6;
			int sCol = (windowCols / 2) - ((menuCols + 8 > 25 ? menuCols + 8 : 25) / 2) + 1;
			int eCol = (windowCols / 2) + ((menuCols + 8 > 25 ? menuCols + 8 : 25) / 2) - 8;
			if (fo == 1) {
				if (has_colors())
					attron(COLOR_PAIR(2));
				attron(A_BOLD);
				mvaddstr(sRow, sCol, "< select >");
				attroff(A_BOLD);
				if (has_colors())
					attron(COLOR_PAIR(1));
				mvaddstr(sRow, eCol, "< exit >");
			} else {
				mvaddstr(sRow, sCol, "< select >");
				if (has_colors())
					attron(COLOR_PAIR(2));
				attron(A_BOLD);
				mvaddstr(sRow, eCol, "< exit >");
				attroff(A_BOLD);
				if (has_colors())
					attron(COLOR_PAIR(1));
			}
		}
	}
}

/*
 * Gets the number of menu options (i.e. rows)
 * from the global menu[] array. This defines
 * the height of the menu, and is needed for
 * centering and drawing borders.
 */
int menu_rows(char **menu) {
	int rows;

	for (rows = 0; menu[rows] && rows < MAX_MENU_OPTIONS; ++rows)
		;

	return rows;
}

/*
 * Gets the number of characters (i.e. cols) in
 * the longest menu option from the global menu[]
 * array. This defines the width of the menu, and
 * is needed for centering and drawing borders.
 */
int menu_cols(char **menu) {
	int rows, cols;
	int longest = 0;

	for (rows = 0; menu[rows] && rows < MAX_MENU_OPTIONS; ++rows) {
		cols = strlen(menu[rows]);
		if (longest < cols)
			longest = cols;
	}

	return longest;
}
