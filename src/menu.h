#ifndef MENU_H
#define MENU_H 1

#define MENU_TITLE           "Select Option"
#define MAX_MENU_OPTIONS     10

void menu_header(int, char *);
void menu_decorate(char **, char *, int, int);
void menu_print(char **, int, int, int, int);
int  menu_rows(char **);
int  menu_cols(char **);

#endif
