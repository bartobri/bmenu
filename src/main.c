// Copyright (c) 2016 Brian Barto
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.  See COPYING for more details.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "config.h"
#include "menu.h"

#define VERSION              "0.1.1"
#define ENTER                10

/***************************************************
 * Main function
 *
 * Returns non-zero value on error. Check stderr for
 * more info.
 ***************************************************/
int main (int argc, char *argv[]) {
	int c, lo = 1, fo = 1;
	char *menuTitle                            = MENU_TITLE;
	char *configFile                           = MENU_CONFIG;
	char **menu;
	char **command;

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
	switch (config_load(configFile)) {
		case 1:
			fprintf(stderr, "Please set HOME environment variable.\n");
			return 1;
			break;
		case 2:
			fprintf(stderr, "Could not open config file: %s\n", configFile);
			return 2;
			break;
		case 3:
			fprintf(stderr, "Memory allocation error. Could not open config file: %s\n", configFile);
			return 3;
			break;
		case 4:
			fprintf(stderr, "Invalid line format detected in config file: %s.\n", configFile);
			return 4;
			break;
	}
	
	// Get menu and command settings from the config file
	menu = config_get_menu();
	command = config_get_command();
	
	// Initialize and start ncurses
	menu_init();

	// Print window header
	menu_header(VERSION);

	// Menu title and borders
	menu_decorate(menu, menuTitle);

	// Print menu with first option highlighted
	menu_print(menu, lo, fo);
	
	// Input loop
	while ((c = getch()) != ENTER) {
		switch (c) {
			case KEY_UP:
			case 107: // 107 == k
				if (lo > 1)
					--lo;
				break;
			case KEY_DOWN:
			case 106: // 106 == j
				if (lo < menu_rows(menu))
					++lo;
				break;
			case KEY_RIGHT:
			case 108: // 108 == l
				fo = 2;
				break;
			case KEY_LEFT:
			case 104: // 104 == h
				fo = 1;
				break;
		}
		
		// Update menu with new selection
		menu_print(menu, lo, fo);
	}

	// End menu display
	menu_cleanup();

	// Execute chosen command
	if (fo == 1)
		execl("/bin/sh", "/bin/sh", "-c", command[lo - 1], (char *) NULL);

	// Freeing memory used for menu[]
	int row = 0;
	while (menu[row])
		free(menu[row++]);

	return 0;
}
