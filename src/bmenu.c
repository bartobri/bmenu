// Copyright (c) 2016 Brian Barto
// 
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.  See COPYING for more details.

#include <stdio.h>
// #define NCURSES_WIDECHAR 1
#include <ncurses.h>
#include <string.h>
#include <ctype.h>           // isspace()
#include <stdlib.h>          // getenv()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>          // execl(), getopt()
#include <stdbool.h>         // bool, true, false

#define VERSION              "0.1.0"

#define MENU_TITLE           "Select Option"
#define MENU_CONFIG          ".bmenu"

#define MAX_MENU_OPTIONS     10

#define SPACE                32
#define NEWLINE              10
#define ENTER                10

// Function prototypes
int loadMenuConfig(char **, char **, char *);
void windowHeader(int);
void decorateMenu(char **, char *, int, int);
void printMenu(char **, int, int, int, int);
int getMenuRows(char **);
int getMenuCols(char **);
void createConfig(char *);
int fileExists(char *);

/***************************************************
 * Main function
 *
 * Returns non-zero value on error. Check stderr for
 * more info.
 ***************************************************/
int main (int argc, char *argv[]) {
	int c, row, col, windowRows, windowCols;
	char *menu[MAX_MENU_OPTIONS]               = {0};
	char *command[MAX_MENU_OPTIONS]            = {0};
	char *menuTitle                            = MENU_TITLE;
	char *configFile                           = MENU_CONFIG;

	// Processing command arguments
	while ((c = getopt(argc, argv, "t:c:")) != -1) {
		switch (c) {
			case 't':
				menuTitle = optarg;
				break;
			case 'c':
				configFile = optarg;
				break;
		}
	}

	// Getting menu config
	int result = loadMenuConfig(menu, command, configFile);
	if (result == 1) {
		fprintf(stderr, "Please set HOME environment variable.\n");
		return result;
	} else if (result == 2) {
		fprintf(stderr, "Could not open config file: %s\n", configFile);
		return result;
	} else if (result == 3) {
		fprintf(stderr, "Memory allocation error. Could not open config file: %s\n", configFile);
		return result;
	} else if (result == 4) {
		fprintf(stderr, "Invalid line format detected in config file: %s.\n", configFile);
		return result;
	}

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

	// Get terminal window size
	getmaxyx(stdscr, windowRows, windowCols);

	// Print window header
	windowHeader(windowCols);

	// Menu title and borders
	decorateMenu(menu, menuTitle, windowRows, windowCols);

	// Menu loop
	int menuListOption = 1, menuFootOption = 1;
	int input = 0;
	int menuRows = getMenuRows(menu);
	do {
		// Check input
		switch(input) {
			case KEY_UP:
				if (menuListOption > 1)
					--menuListOption;
				break;
			case KEY_DOWN:
				if (menuListOption < menuRows)
					++menuListOption;
				break;
			case KEY_RIGHT:
				menuFootOption = 2;
				break;
			case KEY_LEFT:
				menuFootOption = 1;
				break;
			case 106: // 106 == j
				if (menuListOption < menuRows)
				    ++menuListOption;
				break;
			case 107: // 107 == k
				if (menuListOption > 1)
				    --menuListOption;
				break;
			case 104: // 104 == h
				menuFootOption = 1;
				break;
			case 108: // 108 == l
				menuFootOption = 2;
				break;
		}

		// Print menu with the current selection highlighted
		printMenu(menu, menuListOption, menuFootOption, windowRows, windowCols);

		// Refresh window
		refresh();

	} while ((input = getch()) != ENTER);

	// End curses mode
	endwin();

	// Execute chosen command
	if (menuFootOption == 1)
		execl("/bin/sh", "/bin/sh", "-c", command[menuListOption - 1], (char *) NULL);

	// Freeing memory used for menu[]
	row = 0;
	while (menu[row])
		free(menu[row++]);

	return 0;
}

/******************************************************
 * loadMenuConfig()
 *
 * Loading the menu config file. Return a non-zero
 * result if anything goes wrong.
 *
 * Args:
 * char **menu - array of pointers to menu strings
 * char **command - array of pointers to command strings
 * char *config - Config file path
 ******************************************************/
int loadMenuConfig(char **menu, char **command, char *config) {
	char *menuConfigPath;

	// Lets get the config file path. If it is the same as MENU_CONFIG (i.e. default)
	// then we need to build the full path from the $HOME env variable. Otherwise,
	// the full path should already be provided.
	if (strcmp(config, MENU_CONFIG) == 0) {

		char *homeDir = getenv("HOME");

		if (homeDir == NULL)
			return 1;

		menuConfigPath = malloc(strlen(homeDir) + strlen(config) + 2);

		if (menuConfigPath == NULL)
			return 3;

		strcpy(menuConfigPath, homeDir);
		strcat(menuConfigPath, "/");
		strcat(menuConfigPath, config);

		if (!fileExists(menuConfigPath))
			createConfig(menuConfigPath);
	} else {
		menuConfigPath = malloc(strlen(config) + 1);

		if (menuConfigPath == NULL)
			return 3;

		strcpy(menuConfigPath, config);
	}

	// Open file
	FILE *menuConfig = fopen(menuConfigPath, "r");

	// Free memory previously allocated for file path
	free(menuConfigPath);

	// If there was an issue opening the file, return to main()
	if (menuConfig == NULL)
		return 2;

	// Looping over the file, reading in menu and command items
	int l = 0;
	char *confline = NULL;
	size_t linelen = 0;
	while (getline(&confline, &linelen, menuConfig) != -1) {

		// Skipping empty lines
		int i = 0;
		while (isspace(confline[i]))
			++i;
		if (i == strlen(confline))
			continue;

		// Parsing confline
		menu[l] = confline;
		strtok(confline, ":");
		command[l] = strtok(NULL, "\n");

		// If command[l] is NULL then the line is not formatted correctly.
		// Free menu[] and return to caller with appropriate value.
		if (command[l] == NULL) {

			// Freeing memory used for menu[]
			int row = 0;
			while (menu[row])
				free(menu[row++]);

			// Close the open file handle
			fclose(menuConfig);

			return 4;
		}

		l++;
		confline = NULL;
	}

	// Need to free confline in case the last line of the config file was blank
	// resulting in a dangling confline buffer.
	free(confline);

	// Let's close the file handle.
	fclose(menuConfig);

	return 0;
}

/*************************************************
 * createConfig()
 *
 * Creates default config file at given file path
 *
 * Args:
 * char *menuDefaultPath - file path
 *************************************************/
void createConfig(char *menuDefaultPath) {
	FILE *menu = fopen(menuDefaultPath, "w");

	// Return if we can't open the file. loadMenuConfig will
	// ultimately return an error code to main which will
	// terminate the program with an error message.
	if (menu == NULL) 
		return;

	fprintf(menu, "Clear Screen:/bin/clear\n");
	fprintf(menu, "Dir Listing:/bin/ls -l");
	fclose(menu);
}

/****************************************************
 * windowHeader()
 *
 * Prints the window header (title and crossbar).
 *
 * Args:
 * int windowCols - number of cols in terminal window
 ****************************************************/
void windowHeader(int windowCols) {
	int col, textRow = 0, barRow = 1;

	attron(A_BOLD);
	mvprintw(textRow, 1, "%s", "B-MENU v" VERSION);
	attroff(A_BOLD);

	move(1,0);
	hline(ACS_HLINE, windowCols);
}

/****************************************************
 * decorateMenu()
 *
 * Prints the inner and outer borders for the menu.
 *
 * Args:
 * char **menu - array of pointers to menu strings
 * char *title - Menu title
 * int windowRows - number of rows in terminal window
 * int windowCols - number of cols in terminal window
 ****************************************************/
void decorateMenu(char **menu, char *title, int windowRows, int windowCols) {
	int borderCols, borderRows, startRow, startCol;
	int row, col;

	// Border size (inner)
	borderCols = getMenuCols(menu) + 8;
	borderRows = getMenuRows(menu) + 4;

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

/****************************************************
 * printMenu()
 *
 * Prints the menu options list and the select/exit
 * options. Also highlights the current selected
 * options. 
 * 
 * Args:
 * char **menu - array of pointers to menu strings
 * int lo - List Option (currently selected)
 * int fo - Foot Option (currently selected)
 * int windowRows - number of rows in terminal window
 * int windowCols - number of cols in terminal window
 ****************************************************/
void printMenu(char **menu, int lo, int fo, int windowRows, int windowCols) {
	int row, startRow, startCol;
	int menuRows = getMenuRows(menu);
	int menuCols = getMenuCols(menu);

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
			if ((fo == 1)) {
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

/*************************************************
 * getMenuRows()
 *
 * Gets the number of menu options (i.e. rows)
 * from the global menu[] array. This defines
 * the height of the menu, and is needed for
 * centering and drawing borders.
 *
 * Args:
 * char **menu - array of pointers to menu strings
 *
 * Return:
 * int - Number of menu rows
 *************************************************/
int getMenuRows(char **menu) {
	int rows;

	for (rows = 0; menu[rows] && rows < MAX_MENU_OPTIONS; ++rows)
		;

	return rows;
}

/*************************************************
 * getMenuCols()
 *
 * Gets the number of characters (i.e. cols) in
 * the longest menu option from the global menu[]
 * array. This defines the width of the menu, and
 * is needed for centering and drawing borders.
 *
 * Args:
 * char **menu - array of pointers to menu strings
 *
 * Return:
 * int - Number of characters in longest option
 *************************************************/
int getMenuCols(char **menu) {
	int rows, cols;
	int longest = 0;

	for (rows = 0; menu[rows] && rows < MAX_MENU_OPTIONS; ++rows) {
		cols = strlen(menu[rows]);
		if (longest < cols)
			longest = cols;
	}

	return longest;
}

/*************************************************
 * fileExists()
 *
 * Checks if given file path exists.
 *
 * Return:
 * int - TRUE = exists, FALSE = doesn't exist
 *************************************************/
int fileExists(char *filename)
{
	struct stat buffer;   
	return (stat (filename, &buffer) == 0);
}
