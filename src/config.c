#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include "config.h"

#define MAX_MENU_LENGTH    100
#define MAX_COMMAND_LENGTH 1000

static char *menu[MAX_MENU_OPTIONS];
static char *command[MAX_MENU_OPTIONS];
static int menu_count;


/*
 * Load the menu config file. Return a non-zero result if anything goes
 * wrong.
 */
int config_load(char *config) {
	int i, j;
	char c;
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
		menuConfigPath = config;
	}

	// Open file
	FILE *menuConfig = fopen(menuConfigPath, "r");

	// If there was an issue opening the file, return to main()
	if (menuConfig == NULL)
		return 2;
		
	// Free memory previously allocated for file path
	if (strcmp(config, MENU_CONFIG) == 0)
		free(menuConfigPath);

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

/*
 * free all allocated memory
 */
void config_free_all(void) {
	int i;

	for (i = 0; i <= menu_count; ++i) {
		free(menu[i]);
		free(command[i]);
	}
}



