// Copyright (c) 2017 Brian Barto
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.  See LICENSE for more details.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "menu.h"
#include "tio.h"

// Menu option size limits
#define MAX_MENU_OPTIONS   10
#define MAX_MENU_LENGTH    100
#define MAX_COMMAND_LENGTH 1000

// Menu Border Characters
#define INNER_ULCORNER     "\xe2\x94\x8C"
#define INNER_URCORNER     "\xe2\x95\x96"
#define INNER_LLCORNER     "\xe2\x95\x98"
#define INNER_LRCORNER     "\xe2\x95\x9D"
#define OUTER_ULCORNER     "\xe2\x95\x94"
#define OUTER_URCORNER     "\xe2\x95\x95"
#define OUTER_LLCORNER     "\xe2\x95\x99"
#define OUTER_LRCORNER     "\xe2\x94\x98"
#define OUTER_LTEE         "\xe2\x95\xA0"
#define OUTER_RTEE         "\xe2\x95\xA1"
#define HLINE              "\xe2\x94\x80"
#define DHLINE             "\xe2\x95\x90"
#define VLINE              "\xe2\x94\x82"
#define DVLINE             "\xe2\x95\x91"
#define SHADE              "\xe2\x96\x91"
#define ARROW              "\xe2\x96\xBA"

// Static Variables
static char *menu_title = "Select Option";
static char *menu_config = ".bmenu";
static char *menu[MAX_MENU_OPTIONS];
static char *command[MAX_MENU_OPTIONS];
static int  menu_count = 0;

// Static Function Prototypes
static void menu_create(char *);
static int  menu_exists(char *);
static int  menu_max_cols(void);
static void menu_print_header(char *);
static void menu_print_border(void);
static void menu_print_options(int, int);

/*
 * Initialize and configure the terminal for output.
 */
void menu_init(void) {
	tio_init_terminal();
}

/*
 * Restore terminal settings.
 */
void menu_end(void) {
	tio_restore_terminal();
}

/*
 * Set menu title variable.
 */
void menu_set_title(char *title) {
	menu_title = malloc(strlen(title) + 1);
	strcpy(menu_title, title);
}

/*
 * Set config file location.
 */
void menu_set_config(char *config) {
	menu_config = malloc(strlen(config) + 1);
	strcpy(menu_config, config);
}

/*
 * Load the menu config file. Return a non-zero result if anything goes
 * wrong.
 */
int menu_load(void) {
	int i, j;
	char *config = menu_config;
	int c;
	char *menuConfigPath;

	// If the menu_config variable starts with a backslash then we use it
	// as is, assuming it is a full path. If it does not start with a
	// backslash, then we need to build the full path from the $HOME env
	// variable.
	if (*config == '/') {
		menuConfigPath = config;
	} else {

		char *homeDir = getenv("HOME");

		if (homeDir == NULL)
			return 1;

		menuConfigPath = malloc(strlen(homeDir) + strlen(config) + 2);

		if (menuConfigPath == NULL)
			return 3;

		strcpy(menuConfigPath, homeDir);
		strcat(menuConfigPath, "/");
		strcat(menuConfigPath, config);

		if (!menu_exists(menuConfigPath))
			menu_create(menuConfigPath);
	}

	// Open file
	FILE *menuConfig = fopen(menuConfigPath, "r");

	// If there was an issue opening the file, return to main()
	if (menuConfig == NULL)
		return 2;

	// Loop over config file and store menu items
	for (i = 0; i < MAX_MENU_OPTIONS; ++i) {
		
		// Allocate menumemory
		menu[i] = malloc(MAX_MENU_LENGTH);
		memset(menu[i], 0, MAX_MENU_LENGTH);
		
		// Allocate command memory
		command[i] = malloc(MAX_COMMAND_LENGTH);
		memset(command[i], 0, MAX_COMMAND_LENGTH);
		
		// Getting menu text
		for (j = 0; (c = fgetc(menuConfig)) != EOF && c != ':' && c != '\n'; ++j) {
			menu[i][j] = c;
		}
		if (c == '\n') {
			free(menu[i]);
			free(command[i]);
			--i;
			continue;
		}
		if (c == EOF) {
			break;
		}
		
		// Getting menu command
		for (j = 0; (c = fgetc(menuConfig)) != EOF && c != '\n'; ++j) {
			command[i][j] = c;
		}
		if (c == EOF) {
			break;
		}
	}
	
	// Keep track of menu count
	menu_count = i;

	return 0;
}
/*
 * Return the number of menu items loaded by menu_load().
 */
int menu_get_count(void) {
	return menu_count;
}

/*
 * Return the menu file path used by menu_load().
 */
char *menu_get_config_path(void) {
	return menu_config;
}

/*
 * Display the menu to the user. lo and fo represent the list option and
 * the foot option that should be highlighted.
 */
void menu_show(char *version, int lo, int fo) {
	// print menu header
	menu_print_header(version);

	// print inner border
	menu_print_border();
	
	// print menu options
	menu_print_options(lo, fo);
}

/*
 * Execute the command at index lo.
 */
void menu_execute(int lo) {
	execl("/bin/sh", "/bin/sh", "-c", command[lo - 1], (char *) NULL);
}

/*
 * Free all allocated memory for menu and command arrays.
 */
void menu_free_all(void) {
	int i;

	for (i = 0; i < menu_count; ++i) {
		free(menu[i]);
		free(command[i]);
	}
}

///////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
///////////////////////////////////////////////////////////////////////

/*
 * Create a new menu configuration file
 */
static void menu_create(char *path) {
	FILE *menu = fopen(path, "w");

	// Return if we can't open the file. menu_load() will
	// ultimately return an error code to main which will
	// terminate the program with an error message.
	if (menu == NULL)
		return;

	fprintf(menu, "Clear Screen:/bin/clear\n");
	fprintf(menu, "Dir Listing:/bin/ls -l");
	fclose(menu);
}

/*
 * Check if a menu file exists at the given path.
 */
static int menu_exists(char *path) {
	struct stat buffer;
	return (stat(path, &buffer) == 0);
}

/*
 * Return the menu column width.
 */
static int menu_max_cols(void) {
	int i, l;

	for (l = 0, i = 0; i < menu_count; ++i)
		if (l < (int)strlen(menu[i]))
			l = strlen(menu[i]);

	return l;
}

/*
 * Print menu header.
 */
static void menu_print_header(char *v) {
	int i;
	int term_cols = tio_get_cols();
	
	tio_move_cursor(1, 2);
	
	tio_set_text_normal();
	tio_set_text_bold();
	printf("B-MENU v%s", v);
	tio_set_text_normal();
	
	tio_move_cursor(2, 1);
	
	for (i = 0; i < term_cols; ++i)
		printf("%s", DHLINE);
}

/*
 * Print menu borders.
 */
static void menu_print_border(void) {
	int i, j;
	int borderCols, borderRows, startCol, startRow;
	int term_cols = tio_get_cols();
	int term_rows = tio_get_rows();
	int menu_cols = menu_max_cols();
	
	// Inner border size
	borderCols = menu_cols + 8;
	borderRows = menu_count + 4;
	if (borderCols < 25)
		borderCols = 25;
	startCol = (term_cols / 2) - (borderCols / 2);
	startRow = (term_rows / 2) - (borderRows / 2);
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;
	
	// Printing inner border
	for (i = 0; i < borderRows; ++i) {
		for (j = 0; j < borderCols; ++j) {
			if (i == 0 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", INNER_ULCORNER);
			} else if (i == 0 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", INNER_URCORNER);
			} else if (i == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", HLINE);
			} else if (i == borderRows - 1 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", INNER_LLCORNER);
			} else if (i == borderRows - 1 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", INNER_LRCORNER);
			} else if (i == borderRows - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", DHLINE);
			} else if (j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", VLINE);
			} else if (j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", DVLINE);
			}
		}
	}
	
	// Printing inner border title
	tio_set_text_bold();
	tio_move_cursor(startRow - 1, startCol);
	printf("%s", menu_title);
	tio_set_text_normal();
	
	// Outer border size
	borderCols += 4;
	borderRows += 4;
	startCol = ((term_cols / 2) - (borderCols / 2));
	startRow = ((term_rows / 2) - (borderRows / 2));
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;
	borderRows += 3;          // Extra rows for select/exit options
	
	// printing border (outer)
	for (i = 0; i < borderRows; ++i) {
		for (j = 0; j < borderCols; ++j) {
			if (i == 0 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", OUTER_ULCORNER);
			} else if (i == 0 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", OUTER_URCORNER);
			} else if (i == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", DHLINE);
			} else if (i == borderRows - 1 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", OUTER_LLCORNER);
			} else if (i == borderRows - 1 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", OUTER_LRCORNER);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%s", SHADE);
			} else if (i == borderRows - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", HLINE);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%s", SHADE);
			} else if (j == 0 && i == borderRows - 3) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", OUTER_LTEE);
			} else if (j == borderCols - 1 && i == borderRows - 3) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", OUTER_RTEE);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%s", SHADE);
			} else if (j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", DVLINE);
			} else if (j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", VLINE);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%s", SHADE);
			} else if (i == borderRows - 3) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%s", DHLINE);
			}
		}
	}
}

/*
 * Print menu options with current selections highlighted as indicated
 * by lo and fo (list option and foot option).
 */
static void menu_print_options(int lo, int fo) {
	int i, startCol, startRow, sCol, eCol;
	int term_cols = tio_get_cols();
	int term_rows = tio_get_rows();
	int menu_cols = menu_max_cols();
	
	// Menu starting position
	startCol = ((term_cols / 2) - (menu_cols / 2));
	startRow = ((term_rows / 2) - (menu_count / 2));
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;

	// Inserting menu in to terminal window
	for (i = 0; i < menu_count; ++i) {

		// Printing selection marker if on selected row, and removing any previous
		// marker if not.
		if (i == lo - 1) {
			tio_set_text_highlight();
			tio_move_cursor(i + startRow, startCol - 2);
			printf("%s", ARROW);
			tio_move_cursor(i + startRow, startCol);
			printf("%s", menu[i]);
			tio_set_text_normal();
		} else {
			tio_move_cursor(i + startRow, startCol - 2);
			printf(" ");
			tio_move_cursor(i + startRow, startCol);
			printf("%s", menu[i]);
		}

		// printing menu foot options (select/exit)
		if (i == menu_count - 1) {
			sCol = (term_cols / 2) - ((menu_cols + 8 > 25 ? menu_cols + 8 : 25) / 2) + 1;
			eCol = (term_cols / 2) + ((menu_cols + 8 > 25 ? menu_cols + 8 : 25) / 2) - 8;
			if (fo == 1) {
				tio_set_text_highlight();
				tio_move_cursor(i + startRow + 6, sCol);
				printf("< select >");
				tio_set_text_normal();
				tio_move_cursor(i + startRow + 6, eCol);
				printf("< exit >");
			} else {
				tio_move_cursor(i + startRow + 6, sCol);
				printf("< select >");
				tio_set_text_highlight();
				tio_move_cursor(i + startRow + 6, eCol);
				printf("< exit >");
			}
		}
	}
}
