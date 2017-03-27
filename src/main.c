// Copyright (c) 2016 Brian Barto
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.  See COPYING for more details.

#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>          // execl(), getopt()
#include <stdbool.h>

#include "config.h"
#include "menu.h"

#define VERSION              "0.1.1"

#define NEWLINE              10
#define ENTER                10

// Function prototypes
void windowHeader(int);
void decorateMenu(char **, char *, int, int);
void printMenu(char **, int, int, int, int);
int getMenuRows(char **);
int getMenuCols(char **);

/***************************************************
 * Main function
 *
 * Returns non-zero value on error. Check stderr for
 * more info.
 ***************************************************/
int main (int argc, char *argv[]) {
	int c, row, windowRows, windowCols;
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
	int result = config_load(menu, command, configFile);
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
	menu_header(windowCols, VERSION);

	// Menu title and borders
	menu_decorate(menu, menuTitle, windowRows, windowCols);

	// Menu loop
	int menuListOption = 1, menuFootOption = 1;
	int input = 0;
	int menuRows = menu_rows(menu);
	do {
		// Check input
		switch(input) {
			case KEY_UP:
			case 107: // 107 == k
				if (menuListOption > 1)
					--menuListOption;
				break;
			case KEY_DOWN:
			case 106: // 106 == j
				if (menuListOption < menuRows)
					++menuListOption;
				break;
			case KEY_RIGHT:
			case 108: // 108 == l
				menuFootOption = 2;
				break;
			case KEY_LEFT:
			case 104: // 104 == h
				menuFootOption = 1;
				break;
		}

		// Print menu with the current selection highlighted
		menu_print(menu, menuListOption, menuFootOption, windowRows, windowCols);

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
