#ifndef RPEXPOSE_H
#define RPEXPOSE_H

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/xpm.h>


/* parse.c */
#define BUFFER_SIZE			1024
#define SMALL_BUFFER_SIZE	128
#define ARGV_SIZE			64

/* gui.c */
#define NORMAL_WIDTH		4
#define NORMAL_HEIGHT	3
#define WIDE_WIDTH		16
#define WIDE_HEIGHT		9

/* thumbnail.c */
#define THUMB_WIDTH		128
#define THUMB_HEIGHT		128
#define THUMB_PADDING	32
#define THUMB_OFFSET		0
#define THUMB_BPL			(4*THUMB_WIDTH)
#define THUMB_SIZE		(THUMB_BPL*THUMB_HEIGHT)

typedef enum {A_UNKNOWN=0, A_CLEAR, A_GENERATE, A_SELECT} action_t;
typedef enum {S_STARTUP, S_RUNNING, S_INSERT, S_SHUTDOWN} status_t;

typedef struct _thumbnail thumbnail_t;

struct _thumbnail{
	XImage *image;

	int width, height, x, y;

	thumbnail_t *left, *right;

	Window xid;
	
	char *name, *id;
};

typedef struct _patricia patricia_t;

struct _patricia{
	patricia_t *children[10];
	thumbnail_t *window;
};

typedef struct _global global_t;

struct _global{
	status_t status;
	action_t action;

	struct{
		XFontStruct *font;
		int height, width, x, y;
		char buffer[BUFFER_SIZE];
		int length;
		int cursor;
	} colon;

	struct{	
		char *name;
		FILE *handle;

		char *home;
	} file;

	struct{
		int width, height;

		int num_thumbs;
		thumbnail_t *thumbs;
		thumbnail_t *selected;
	} gui;

	struct{
		patricia_t top;
		patricia_t *selected;
	} p;
	
	struct{
		Display *display;
		Window window;
		Pixmap buffer;
		int width, height;
		GC gc;
		GC rgc;
	} x;

	struct{
		char *select_exec, *colon_exec;

		int widescreen;

		int text_padding, border_padding, thumb_padding;

		char *keybindings[256];
	} rc;
};

global_t g;

typedef enum{LEFT, RIGHT, UP, DOWN} direction_t;

/* parse.c */
int load_rcfile();

int load_rcdefaults();

int parse_command(char *command);

int parse_set(char *command);

int parse_bind(char *command, int unbind);

char *parse_split(char *command);

/* rpexpose.c */
int rpexpose_clean();

int rpexpose_generate();

int rpexpose_select();

void clean_up();

/* gui.c */
int load_xwindow();

int load_input();

int load_buffer();

int event_draw();

int event_redraw(int x, int y, int width, int height);

int event_move(thumbnail_t *n);

int event_select();

void patricia_insert(char *id, thumbnail_t *window);

/* thumbnail.c */
XImage *thumbnail_generate(Window window);

int thumbnail_write(XImage *thumbnail, char *filename);

XImage *thumbnail_read(char *filename);

#endif

