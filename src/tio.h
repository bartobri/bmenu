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
