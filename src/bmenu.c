// Copyright (c) 2016 Brian Barto
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     // getenv()
#include <sys/ioctl.h>  // Support for terminal dimentions
#include <termios.h>    // Support for character input
#include <unistd.h>     // Support for character input

#define MENU_CONFIG		".bmenu"

#define MAX_MENU_OPTIONS	10
#define MAX_MENU_CHAR		50
#define MAX_COMMAND_CHAR	100

#define SPACE	040
#define NEWLINE 012
#define ENTER	012

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// Making menu vars global
char menu[MAX_MENU_OPTIONS][MAX_MENU_CHAR];
char command[MAX_MENU_OPTIONS][MAX_COMMAND_CHAR];
int menuRows = 0, menuCols = 0;

// Making window dimentions global
int windowRows, windowCols;


/***************************
 * Main function
 ***************************/
int main (void) {
	int loadMenuConfig(void);
	void printOuterHeader(void);
	void printMenu(int);

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

	// Outer header
	printOuterHeader();

	// Setting terminal input mode to turn off echo and buffering
	static struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt);			// get terminal parameters, store in oldt
	newt = oldt;                                // copy settings to newt
	newt.c_lflag &= ~(ICANON | ECHO);			// unset ICANON and ECHO
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);   // run new terminal settings

	// Menu loop
	int menuCurOption = 1;
	int input = 0;
	do {
		// Check input
		if (input == 27) {
			input = getchar();
			input = getchar();
			if (input == 65 && menuCurOption > 1)
					--menuCurOption;
			else if (input == 66 && menuCurOption < menuRows)
					++menuCurOption;
		}

		// Print menu with the current selection highlighted
		printMenu(menuCurOption);

		// Position cursor at the bottom of the terminal window
		printf("\033[%i;%iH", windowRows, 1);

	} while ((input = getchar()) != ENTER);

	// Restore terminal settings before exiting
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

	// Execute chosen command
	// TODO - handle command switches
	execl(command[menuCurOption - 1], command[menuCurOption - 1], 0);

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

	int tmpCols = 0;
	_Bool menuOn = 1, commandOn = 0;
	int l = 0, i = 0, c, lc;
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
			lc = c;
			continue;
		} else if (c == '\n') {
			menuOn = 1;
			commandOn = 0;
			i = 0;
			lc = c;
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
			++tmpCols;
		} else if (commandOn) {
			command[l][i] = c;
			command[l][i+1] = '\0';
			tmpCols = 0;
		}

		if (menuRows == 0 || lc == '\n')
			++menuRows;
		if (tmpCols > menuCols)
			menuCols = tmpCols;

		lc = c;
		++i;
	}
	
	fclose(menuConfig);

	return 0;
}

void printMenu(int o) {
	int startRow = 1, startCol = 1;

	// Determining starting row and column for menu
	startCol = ((windowCols / 2) - (menuCols / 2)) > 0 ? ((windowCols / 2) - (menuCols / 2)) : 0;
	startRow = ((windowRows / 2) - (menuRows / 2)) > 0 ? ((windowRows / 2) - (menuRows / 2)) : 0;

	// Inserting menu in to terminal window
	for (int row = 0; row < menuRows; ++row) {
		if (row == o - 1)
			printf(KMAG);
		else
			printf(KNRM);
		for (int col = 0; menu[row][col] != '\0'; ++col)
			printf("\033[%i;%iH%c", row + startRow, col + startCol, menu[row][col]);
	}
}

void printOuterHeader(void) {
	int textRow = 1;
	int barRow = 2;
	int barChar = '=';

	printf("\033[%i;%iH%s", textRow, 2, "B-MENU");

	for (int col = 0; col < windowCols; ++col)
		printf("\033[%i;%iH%c", barRow, col, barChar);
}
