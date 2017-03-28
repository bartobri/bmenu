#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "menu.h"
#include "tio.h"

#define MAX_MENU_OPTIONS   10
#define MAX_MENU_LENGTH    100
#define MAX_COMMAND_LENGTH 1000

#define ULCORNER     '+'
#define URCORNER     '+'
#define LLCORNER     '+'
#define LRCORNER     '+'
#define HLINE        '-'
#define DHLINE       '='
#define VLINE        '|'
#define SHADE        '#'
#define LTEE         '+'
#define RTEE         '+'
#define ARROW        '>'

static char *menu_title = "Select Option";
static char *menu_config = ".bmenu";
static char *menu[MAX_MENU_OPTIONS];
static char *command[MAX_MENU_OPTIONS];
static int  menu_count = 0;

static void menu_create(char *);
static int  menu_exists(char *);
static int  menu_max_cols(void);
static void menu_print_header(char *);

void menu_init(void) {
	tio_init_terminal();
}

void menu_end(void) {
	tio_restore_terminal();
}

void menu_set_title(char *title) {
	menu_title = malloc(strlen(title) + 1);
	strcpy(menu_title, title);
}

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
	char c;
	char *menuConfigPath;

	// Lets get the config file path. If it is the same as MENU_CONFIG (i.e. default)
	// then we need to build the full path from the $HOME env variable. Otherwise,
	// the full path should already be provided.
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

int menu_get_count(void) {
	return menu_count;
}

char *menu_get_config_path(void) {
	return menu_config;
}

void menu_show(char *version, int lo, int fo) {
	int i, j;
	int term_cols = tio_get_cols();
	int term_rows = tio_get_rows();
	int menu_cols = menu_max_cols();
	
	// print menu header
	menu_print_header(version);

	/*
	 * Border
	 */

	// Inner border size
	int borderCols = menu_cols + 8;
	int borderRows = menu_count + 4;
	if (borderCols < 25)
		borderCols = 25;
	int startCol = (term_cols / 2) - (borderCols / 2);
	int startRow = (term_rows / 2) - (borderRows / 2);
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;
	
	// Printing inner border
	for (i = 0; i < borderRows; ++i) {
		for (j = 0; j < borderCols; ++j) {
			if (i == 0 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", ULCORNER);
			} else if (i == 0 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", URCORNER);
			} else if (i == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", HLINE);
			} else if (i == borderRows - 1 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", LLCORNER);
			} else if (i == borderRows - 1 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", LRCORNER);
			} else if (i == borderRows - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", HLINE);
			} else if (j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", VLINE);
			} else if (j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", VLINE);
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
				printf("%c", ULCORNER);
			} else if (i == 0 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", URCORNER);
			} else if (i == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", HLINE);
			} else if (i == borderRows - 1 && j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", LLCORNER);
			} else if (i == borderRows - 1 && j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", LRCORNER);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%c", SHADE);
			} else if (i == borderRows - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", HLINE);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%c", SHADE);
			} else if (j == 0 && i == borderRows - 3) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", LTEE);
			} else if (j == borderCols - 1 && i == borderRows - 3) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", RTEE);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%c", SHADE);
			} else if (j == 0) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", VLINE);
			} else if (j == borderCols - 1) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", VLINE);
				tio_move_cursor(i + startRow + 1, j + startCol + 1);
				printf("%c", SHADE);
			} else if (i == borderRows - 3) {
				tio_move_cursor(i + startRow, j + startCol);
				printf("%c", HLINE);
			}
		}
	}
	
	/*
	 * Menu selections
	 */

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
			printf("%c", ARROW);
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
			int sCol = (term_cols / 2) - ((menu_cols + 8 > 25 ? menu_cols + 8 : 25) / 2) + 1;
			int eCol = (term_cols / 2) + ((menu_cols + 8 > 25 ? menu_cols + 8 : 25) / 2) - 8;
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

void menu_free_all(void) {
	int i;

	for (i = 0; i <= menu_count; ++i) {
		free(menu[i]);
		free(command[i]);
	}
}

///////////////////
// STATIC FUNCTIONS
///////////////////

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

static int menu_exists(char *path) {
	struct stat buffer;
	return (stat(path, &buffer) == 0);
}

static int menu_max_cols(void) {
	int i, l;

	for (l = 0, i = 0; i < menu_count; ++i)
		if (l < (int)strlen(menu[i]))
			l = strlen(menu[i]);

	return l;
}

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
		printf("%c", DHLINE);
}
