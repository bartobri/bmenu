#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include "config.h"

static char *menu[MAX_MENU_OPTIONS]               = {0};
static char *command[MAX_MENU_OPTIONS]            = {0};


/*
 * Load the menu config file. Return a non-zero result if anything goes
 * wrong.
 */
int config_load(char *config) {
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

		if (!config_exists(menuConfigPath))
			config_create(menuConfigPath);
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
		size_t i = 0;
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

/*
 * createConfig()
 *
 * Creates default config file at given file path
 *
 * Args:
 * char *menuDefaultPath - file path
 */
void config_create(char *menuDefaultPath) {
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

/*
 * Checks if given file path exists.
 */
int config_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

/*
 * return the menu pointer
 */
char **config_get_menu(void) {
	return menu;
}

/*
 * return the command pointer
 */
char **config_get_command(void) {
	return command;
}



