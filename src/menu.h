#ifndef MENU_H
#define MENU_H 1

#define MENU_TITLE           "Select Option"
#define MAX_MENU_OPTIONS     10

void menu_init(void);
void menu_header(char *);
void menu_decorate(char **, char *);
void menu_print(char **, int, int);
int  menu_rows(char **);
int  menu_cols(char **);

#endif
