// Copyright (c) 2016 Brian Barto
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// getenv()
#include <sys/ioctl.h>		// Support for terminal dimentions
#include <termios.h>		// Support for character input
#include <unistd.h>			// Support for character input

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

///////////////////////////////
// Main function
///////////////////////////////
int main (void) {
	int loadMenuConfig(void);

	// Get terminal size
	int wRows, wCols;
	struct winsize w;                                                                                         
	ioctl(0, TIOCGWINSZ, &w);
	wRows =  w.ws_row;
	wCols = w.ws_col;

	// initializing window with all spaces
	char window[wRows][wCols];

	for (int c1 = 0; c1 < wRows; ++c1) {
		int c2;
		for (c2 = 0; c2 < wCols - 1; ++c2)
			window[c1][c2] = SPACE;
		window[c1][c2] = NEWLINE;
	}

	// Printing (clearing) window
	for (int c1 = 0; c1 < wRows; ++c1)
		for (int c2 = 0; c2 < wCols; ++c2)
			printf("%c", window[c1][c2]);

	// Getting menu config
	int result = loadMenuConfig();
	if (result == 1) {
		fprintf(stderr, "Please set HOME environment variable.\n");
		return result;
	} else if (result == 2) {
		fprintf(stderr, "Could not open config file: ~/" MENU_CONFIG "\n");
		return result;
	}

	// Determining starting row and column
	// TODO - consider longest line length may be longer
	//        than window width resulting in a negative
	//        starting point.
	int startRow, startCol;
	startCol = (wCols / 2) - (menuCols / 2);
	startRow = (wRows / 2) - (menuRows / 2);

	// Inserting menu in to window
	for (int line = 0; line < menuRows; ++line)
		for (int charCount = 0; menu[line][charCount] != '\0'; ++charCount)
			window[line + startRow][(charCount) + startCol] = menu[line][charCount];

	// Menu Outer Head
	window[1][2] = 'B';
	window[1][3] = '-';
	window[1][4] = 'M';
	window[1][5] = 'E';
	window[1][6] = 'N';
	window[1][7] = 'U';
	for (int col = 0; window[2][col] != '\n'; ++col)
		window[2][col] = '=';

	// Menu Inner Head and Border
	int menuColPad = 4;
	int menuRowPad = 2;
	if (startRow - menuRowPad > 1 && startCol - menuColPad > 1) {
		int borderStartRow = startRow - menuRowPad;
		int borderStartCol = startCol - menuColPad;
		int borderEndRow = startRow + menuRows + (menuRowPad - 1);
		int borderEndCol = startCol + menuCols + (menuColPad - 1);
		window[borderStartRow - 1][borderStartCol] = 'C';
		window[borderStartRow - 1][borderStartCol + 1] = 'h';
		window[borderStartRow - 1][borderStartCol + 2] = 'o';
		window[borderStartRow - 1][borderStartCol + 3] = 'o';
		window[borderStartRow - 1][borderStartCol + 4] = 's';
		window[borderStartRow - 1][borderStartCol + 5] = 'e';
		window[borderStartRow - 1][borderStartCol + 7] = 'O';
		window[borderStartRow - 1][borderStartCol + 8] = 'p';
		window[borderStartRow - 1][borderStartCol + 9] = 't';
		window[borderStartRow - 1][borderStartCol + 10] = 'i';
		window[borderStartRow - 1][borderStartCol + 11] = 'o';
		window[borderStartRow - 1][borderStartCol + 12] = 'n';
		for (int i1 = borderStartRow; i1 <= borderEndRow; ++i1)
			for (int i2 = borderStartCol; i2 <= borderEndCol; ++i2)
				if (i1 == borderStartRow && i2 == borderStartCol)
					window[i1][i2] = '=';					// Upper left corner
				else if (i1 == borderStartRow && i2 == borderEndCol)
					window[i1][i2] = '=';					// Upper right corner
				else if (i1 == borderEndRow && i2 == borderStartCol)
					window[i1][i2] = '=';					// Lower left corner
				else if (i1 == borderEndRow && i2 == borderEndCol)
					window[i1][i2] = '=';					// Lower right corner
				else if (i1 == borderStartRow)
					window[i1][i2] = '=';					// Top row
				else if (i1 == borderEndRow)
					window[i1][i2] = '=';					// Bottom row
				else if (i2 == borderStartCol)
					window[i1][i2] = '|';					// Left edge
				else if (i2 == borderEndCol)
					window[i1][i2] = '|';					// Right edge
	}

	// Setting terminal input mode to turn off echo and buffering
	static struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt);						// get terminal parameters, store in oldt
	newt = oldt;											// copy settings to newt
	newt.c_lflag &= ~(ICANON | ECHO);						// unset ICANON and ECHO
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);				// run new terminal settings

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

		// printing window
		for (int c1 = 0; c1 < wRows; ++c1)
			for (int c2 = 0; c2 < wCols; ++c2) {
				printf("\033[%i;%iH", c1, c2);				// Set cursor position
				if (c1 == startRow + (menuCurOption - 1) && c2 > startCol - 1 && c2 < startCol + menuCols) {
					printf(KMAG "%c", window[c1][c2]);
					if (c2 == startCol) {
						printf("\033[%i;%iH", c1, c2 - 2);
						printf(KMAG "*");
					}
				} else
					printf(KNRM "%c", window[c1][c2]);
			}

	} while ((input = getchar()) != ENTER);

	// Restore terminal settings before exiting
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

	// Execute chosen command
	// TODO - handle command switches
	execl(command[menuCurOption - 1], command[menuCurOption - 1], 0);

	return 0;
}

///////////////////////////////////////////////////
// loadMenuConfig() - Loading the menu config file.
// Return a non-zero result if anything goes wrong.
///////////////////////////////////////////////////
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
