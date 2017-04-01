// Copyright (c) 2017 Brian Barto
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.  See LICENSE for more details.

#ifndef TIO_H
#define TIO_H 1

void tio_init_terminal(void);
void tio_restore_terminal(void);
int  tio_get_rows(void);
int  tio_get_cols(void);
void tio_set_text_normal(void);
void tio_set_text_highlight(void);
void tio_set_text_bold(void);
void tio_move_cursor(int, int);

#endif
