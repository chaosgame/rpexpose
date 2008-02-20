/*
 * Copyright (c) 2008, Nathan Lawrence
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

#include "assert.h"

#define INTERPOLATE

typedef enum {A_UNKNOWN=0, A_CLEAR, A_GENERATE, A_SELECT} action_t;
typedef enum {S_STARTUP, S_RUNNING, S_SHUTDOWN} status_t;

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
		char *name, *home;
		FILE *handle;
	} file;

	struct{
		int width, height, num_thumbs;
		thumbnail_t *thumbs, *selected;
	} gui;

	struct{
		patricia_t top, *selected;
	} p;
	
	struct{
		Display *display;
		Window window;
		Pixmap buffer;
		int width, height;
		GC gc, rgc;
	} x;

	struct{
		char *select_exec;

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

