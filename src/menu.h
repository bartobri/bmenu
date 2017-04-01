// Copyright (c) 2017 Brian Barto
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.  See LICENSE for more details.

#ifndef MENU_H
#define MENU_H 1

void menu_init(void);
void menu_end(void);
void menu_set_title(char *);
void menu_set_config(char *);
int  menu_load(void);
int  menu_get_count(void);
char *menu_get_config_path(void);
void menu_show(char *, int, int);
void menu_execute(int);
void menu_free_all(void);

#endif
