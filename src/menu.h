#ifndef MENU_H
#define MENU_H 1

void menu_init(void);
void menu_end(void);
void menu_set_title(char *);
void menu_set_config(char *);
int  menu_load(void);
int  menu_get_count(void);
char *menu_get_config_path(void);
void menu_show(char *);
void menu_free_all(void);

#endif
