#include <stdio.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Macros for VT100 codes
#define CLEAR_SCR()          printf("\033[2J")           // Clear Screen
#define CURSOR_HOME()        printf("\033[H")            // Move cursor to home position (0,0)
#define CURSOR_MOVE(y,x)     printf("\033[%i;%iH", y, x) // Move cursor to x,y
#define BEEP()               printf("\a");               // terminal bell
#define BOLD()               printf("\033[1m")           // Cursor bold
#define FOREGROUND_COLOR(x)  printf("\033[3%im", x)      // Set foreground color
#define CLEAR_ATTR()         printf("\033[0m")           // Clear bold/color attributes
#define SCREEN_SAVE()        printf("\033[?47h")         // Save screen display
#define SCREEN_RESTORE()     printf("\033[?47l")         // Restore screen to previously saved state
#define CURSOR_SAVE()        printf("\033[s")            // Save cursor position
#define CURSOR_RESTORE()     printf("\033[u")            // Restore cursor position
#define CURSOR_HIDE()        printf("\033[?25l")         // Hide cursor
#define CURSOR_SHOW()        printf("\033[?25h")         // Unhide cursor

// Color identifiers
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

// Static variables
static int foregroundColor = COLOR_CYAN;            // Foreground color setting

// Function prototypes
static void tio_set_terminal(int);

/*
 * Initialize and configure the terminal for output. This means
 * just turning off terminal echo and line buffering.
 */
void tio_init_terminal(void) {

	// Turn off line buffering and echo
	tio_set_terminal(0);
	
	// Save terminal state, clear screen, and home/hide the cursor
	CURSOR_SAVE();
	SCREEN_SAVE();
	CLEAR_SCR();
	CURSOR_HOME();
	CURSOR_HIDE();
}

/*
 * Restore the state of the terminal to the state prior to executing
 * nmstermio_init_terminal(). This means turning on line buffering
 * and echo.
 */
void tio_restore_terminal(void) {
	
	// Restore screen and cursor is clearSrc is set
	SCREEN_RESTORE();
	CURSOR_SHOW();
	CURSOR_RESTORE();
	
	// Turn on line buffering and echo
	tio_set_terminal(1);
}

/*
 * Get and return the number of rows in the current terminal window.
 */
int tio_get_rows(void) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
	return w.ws_row;
}

/*
 * Get and return the number of cols in the current terminal window.
 */
int tio_get_cols(void) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
	return w.ws_col;
}

/*
 * Clear all text attributes and set default foreground color.
 */
void tio_set_text_normal(void) {
	CLEAR_ATTR();
	FOREGROUND_COLOR(foregroundColor);
}
\
/*
 * Set text to bld with alternate foreground color.
 */
void tio_set_text_highlight(void) {
	CLEAR_ATTR();
	FOREGROUND_COLOR(COLOR_MAGENTA);
	BOLD();
}

/*
 * Just set the text to bold.
 */
void tio_set_text_bold(void) {
	BOLD();
}

/*
 * Move terminal cursor to the given x/y coordinates.
 */
void tio_move_cursor(int y, int x) {
	CURSOR_MOVE(y, x);
}

/*
 * Turn off terminal echo and line buffering when passed an integer value
 * that evaluates to true. Restore the original terminal values when passed
 * an integer value that evaluates to false.
 */
static void tio_set_terminal(int s) {
	struct termios tp;
	static struct termios save;
	static int state = 1;
	
	if (s == 0) {
		if (tcgetattr(STDIN_FILENO, &tp) == -1) {
			return;
		}

		save = tp;
		
		tp.c_lflag &=(~ICANON & ~ECHO);
		
		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1) {
			return;
		}
	} else {
		if (state == 0 && tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1)
			return;
	}
	
	state = s;
}
