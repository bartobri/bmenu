#ifndef CONFIG_H
#define CONFIG_H 1

#define MENU_CONFIG ".bmenu"

int  config_load(char **, char **, char *);
void config_create(char *);
int  config_exists(char *);

#endif
