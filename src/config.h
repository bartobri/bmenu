#ifndef CONFIG_H
#define CONFIG_H 1

#define MENU_CONFIG          ".bmenu"
#define MAX_MENU_OPTIONS     10

int  config_load(char *);
void config_create(char *);
int  config_exists(char *);
char **config_get_menu(void);
char **config_get_command(void);
void config_free_all(void);

#endif
