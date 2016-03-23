// Copyright (c) 2016 Brian Barto
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>          // getenv()
#include <sys/ioctl.h>       // Support for terminal dimentions
#include <termios.h>         // Support for character input
#include <unistd.h>          // Support for character input

#define VERSION              "0.10"

#define MENU_CONFIG          ".bmenu"

#define MAX_MENU_OPTIONS     10
#define MAX_MENU_CHAR        50
#define MAX_COMMAND_CHAR     100

#define SPACE                040
#define NEWLINE              012
#define ENTER                012

#define KNRM                 "\x1B[0m"
#define KRED                 "\x1B[31m"
#define KGRN                 "\x1B[32m"
#define KYEL                 "\x1B[33m"
#define KBLU                 "\x1B[34m"
#define KMAG                 "\x1B[35m"
#define KCYN                 "\x1B[36m"
#define KWHT                 "\x1B[37m"

// Global Vars
int windowRows, windowCols;
char menu[MAX_MENU_OPTIONS][MAX_MENU_CHAR];
char command[MAX_MENU_OPTIONS][MAX_COMMAND_CHAR];

// Function prototypes
int getMenuRows(void);
int getMenuCols(void);


/***************************
 * Main function
 ***************************/
int main (void) {
	int loadMenuConfig(void);
	void windowHeader(void);
	void decorateMenu(void);
	void printMenu(int, int);

	// Initialize menu and command arrays
	for (int row = 0; row < MAX_MENU_OPTIONS; ++row) {
		for (int col = 0; col < MAX_MENU_CHAR; ++col)
			menu[row][col] = '\0';
		for (int col = 0; col < MAX_COMMAND_CHAR; ++col)
			command[row][col] = '\0';
	}

	// Getting menu config
	// TODO - see if I can remove the use of globals in loadMenuConfig()
	int result = loadMenuConfig();
	if (result == 1) {
		fprintf(stderr, "Please set HOME environment variable.\n");
		return result;
	} else if (result == 2) {
		fprintf(stderr, "Could not open config file: ~/" MENU_CONFIG "\n");
		return result;
	}

	// Get terminal window size
	struct winsize w;                                                                                         
	ioctl(0, TIOCGWINSZ, &w);
	windowRows =  w.ws_row;
	windowCols = w.ws_col;

	// initializing terminal window with all spaces
	for (int c1 = 0; c1 < windowRows; ++c1) {
		int c2;
		for (c2 = 0; c2 < windowCols - 1; ++c2) {
			printf("\033[%i;%iH", c1, c2);
			printf("%c", SPACE);
		}
		printf("\033[%i;%iH", c1, c2);
		printf("%c", NEWLINE);
	}

	// Print window header
	windowHeader();

	// Menu title, borders, and select/exit options
	decorateMenu();

	// Setting terminal input mode to turn off echo and buffering
	static struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt);            // get terminal parameters, store in oldt
	newt = oldt;                                // copy settings to newt
	newt.c_lflag &= ~(ICANON | ECHO);           // unset ICANON and ECHO
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);   // run new terminal settings

	// Menu loop
	int menuListOption = 1, menuFootOption = 1;
	int input = 0;
	int menuRows = getMenuRows();
	do {
		// Check input
		if (input == 27) {
			input = getchar();
			input = getchar();
			if (input == 65 && menuListOption > 1)
				--menuListOption;
			else if (input == 66 && menuListOption < menuRows)
				++menuListOption;
			else if (input == 68)
				menuFootOption = 1;
			else if (input == 67)
				menuFootOption = 2;
		}

		// Print menu with the current selection highlighted
		printMenu(menuListOption, menuFootOption);

		// Position cursor at the bottom of the terminal window
		printf("\033[%i;%iH", windowRows, 1);

	} while ((input = getchar()) != ENTER);

	// Restore terminal settings before exiting
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

	// Execute chosen command
	// TODO - handle command switches
	if (menuFootOption == 1)
		execl(command[menuListOption - 1], command[menuListOption - 1], 0);

	return 0;
}

/*************************************************
 * loadMenuConfig() - Loading the menu config file.
 * Return a non-zero result if anything goes wrong.
 *************************************************/
int loadMenuConfig(void) {
	char *homeDir = getenv("HOME");;

	if (homeDir == NULL) {
		return 1;
	}

	int len = strlen(homeDir) + strlen("/" MENU_CONFIG);
	char menuConfigPath[len + 1];
	strcpy(menuConfigPath, homeDir);
	strcat(menuConfigPath, "/");
	strcat(menuConfigPath, MENU_CONFIG);
	FILE *menuConfig;
	if ((menuConfig = fopen(menuConfigPath, "r")) == NULL) {
		return 2;
	}

	_Bool menuOn = 1, commandOn = 0;
	int l = 0, i = 0, c;
	while((c = getc(menuConfig)) != EOF) {
		if (menuOn && i == MAX_MENU_CHAR - 1) {
			menuOn = 0;
			continue;
		}
		if (commandOn && i == MAX_COMMAND_CHAR - 1) {
			commandOn = 0;
			continue;
		}
			
		if (c == ':') {
			commandOn = 1;
			menuOn = 0;
			i = 0;
			continue;
		} else if (c == '\n') {
			menuOn = 1;
			commandOn = 0;
			i = 0;
			++l;
			if (l > MAX_MENU_OPTIONS - 1)
				break;
			else {
				menu[l][0] = '\0';
				command[l][0] = '\0';
			}
			continue;
		}

		if (menuOn) {
			menu[l][i] = c;
			menu[l][i+1] = '\0';
		} else if (commandOn) {
			command[l][i] = c;
			command[l][i+1] = '\0';
		}

		++i;
	}
	
	fclose(menuConfig);

	return 0;
}

void windowHeader(void) {
	int textRow = 1;
	int barRow = 2;
	int barChar = '=';

	printf("\033[%i;%iH%s", textRow, 2, "B-MENU v" VERSION);

	for (int col = 0; col < windowCols; ++col)
		printf("\033[%i;%iH%c", barRow, col, barChar);
}

void decorateMenu() {
	int borderCols, borderRows, startRow, startCol;

	// Border size (inner)
	borderCols = getMenuCols() + 8;
	borderRows = getMenuRows() + 4;

	// Minimum border width is 25 cols.
	// Need at least this much for select/exit options.
	if (borderCols < 25)
		borderCols = 25;

	// Determining starting row and column for border (inner)
	startCol = ((windowCols / 2) - (borderCols / 2));
	startRow = ((windowRows / 2) - (borderRows / 2));
	if (startCol < 0)
		startCol = 0;
	if (startRow < 0)
		startRow = 0;

	// printing border (inner)
	for (int row = 0; row < borderRows; ++row)
		for (int col = 0; col < borderCols; ++col)
			if (row == 0)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '=');
			else if (row == borderRows - 1)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '=');
			else if (col == 0)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '|');
			else if (col == borderCols - 1)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '|');

	// Printing (inner) border title
	printf("\033[%i;%iH%s", startRow - 1, startCol, "Select Option");

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
	for (int row = 0; row < borderRows; ++row)
		for (int col = 0; col < borderCols; ++col)
			if (row == 0)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '=');
			else if (row == borderRows - 1)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '=');
			else if (col == 0)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '|');
			else if (col == borderCols - 1)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '|');
			else if (row == borderRows - 3)
				printf("\033[%i;%iH%c", row + startRow, col + startCol, '=');
	
}

void printMenu(int lo, int fo) {
	int startRow, startCol;
	int menuRows = getMenuRows();
	int menuCols = getMenuCols();

	// Determining starting row and column for menu
	startCol = ((windowCols / 2) - (menuCols / 2)) > 0 ? ((windowCols / 2) - (menuCols / 2)) : 0;
	startRow = ((windowRows / 2) - (menuRows / 2)) > 0 ? ((windowRows / 2) - (menuRows / 2)) : 0;

	// Inserting menu in to terminal window
	for (int row = 0; row < menuRows; ++row) {

		// highlighting current selection text
		printf( (row == lo - 1) ? KMAG : KNRM );

		for (int col = 0; menu[row][col] != '\0'; ++col) {

			// Printing selection marker if on selected row, and removing any previous
			// marker if not.
			if (row == lo - 1 && col == 0)
				printf("\033[%i;%iH%c", row + startRow, col + startCol - 2, '*');
			else if (col == 0)
				printf("\033[%i;%iH%c", row + startRow, col + startCol - 2, SPACE);

			// printing menu text
			printf("\033[%i;%iH%c", row + startRow, col + startCol, menu[row][col]);
		}

		// printing menu foot options (select/exit)
		if (row == menuRows - 1) {
			int sRow = row + startRow + 6;
			int sCol = (windowCols / 2) - ((menuCols + 8 > 25 ? menuCols + 8 : 25) / 2) + 1;
			int eCol = (windowCols / 2) + ((menuCols + 8 > 25 ? menuCols + 8 : 25) / 2) - 8;
			printf( (fo == 1) ? KMAG : KNRM );
			printf("\033[%i;%iH%s", sRow, sCol, "[ select ]");
			printf( (fo == 2) ? KMAG : KNRM );
			printf("\033[%i;%iH%s", sRow, eCol, "[ exit ]");
		}

		// Changing back to normal text color
		printf(KNRM);
	}
}

int getMenuRows(void) {
	int rows;

	for (rows = 0; menu[rows][0] != '\0' && rows < MAX_MENU_OPTIONS; ++rows)
		;

	return rows;
}

int getMenuCols(void) {
	int maxCols = 0;

	for (int rows = 0; menu[rows][0] != '\0' && rows < MAX_MENU_OPTIONS; ++rows)
		for (int cols = 1; menu[rows][cols] != '\0' && cols < MAX_MENU_CHAR; ++cols)
			if (maxCols < cols)
				maxCols = cols;

	return maxCols + 1;
}
