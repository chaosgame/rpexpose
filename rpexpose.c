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

#include "rpexpose.h"

extern const int BUFFER_SIZE;

const char *argp_program_version = "rpexpose 0.1.18";

const char *argp_program_bug_address = "<nlawren2@uiuc.edu>";

static char doc[] = "";

static char args_doc[] = "";

static struct argp_option cmdline_options[] = {
	{"clean",		'c', NULL,		0, "Cleans the /var/run/rpexpose directory" },
	{"generate",	'g', "FILE",	0, "Generates ximage dumps from a list of top level windows"},
	{"select",		's', "FILE",	0, "Creates a selection dialog for a list of managed windows"},
	{0}
};

int open_file(char *filename){
	if( strcmp((g.file.name=filename),"-") )
		g.file.handle=fopen(g.file.name,"r");
	else
		g.file.handle=stdin;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state){
	switch(key){
	case 'c':
		g.action=A_CLEAR;
		break;
	case 'g':
		g.action=A_GENERATE;
		open_file(arg);
		break;
	case 's':
		g.action=A_SELECT;
		open_file(arg);
		break;
	case ARGP_KEY_END:
		if( !g.action ){
			argp_usage(state);
			exit(1);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { cmdline_options, parse_opt, args_doc, doc };

int main(int argc, char **argv){
	g.action = A_UNKNOWN;
	g.file.name = NULL;
	g.file.home = getenv("HOME");

	argp_parse(&argp, argc, argv, 0, 0, NULL);
	
	int error=1;
	switch(g.action){
	case A_CLEAR:
		return rpexpose_clean();
	case A_GENERATE:
		error=rpexpose_generate();
		fclose(g.file.handle);
		return error;
	case A_SELECT:
		error=rpexpose_select();
		fclose(g.file.handle);
		return error;
	}
}

int rpexpose_clean(){
	struct dirent *ep;
	
	char filename[BUFFER_SIZE];
	sprintf(filename,"%s/.rpexpose",g.file.home);

	DIR *directory = opendir(filename);

	if(directory){
		while( ep = readdir(directory) ){
			if( *(ep->d_name)=='.' ) continue;
			if( !strcmp(ep->d_name, "default") ) continue;
			sprintf(filename,"%s/.rpexpose/%s",g.file.home,ep->d_name);
			if( remove(filename) ){
				perror("Could not delete file");
				closedir(directory);
				return 1;
			}
		}
	}
	else{
		perror("Couldn't open $HOME/.rpexpose/\n");
		return 1;
	}
	
	closedir(directory);
	return 0;
}

int rpexpose_generate(){
	g.x.display = XOpenDisplay(getenv("DISPLAY"));

	Window window;
	
	char filename[BUFFER_SIZE];

	sprintf(filename,"%s/.rpexpose/",g.file.home);

	DIR *directory = opendir(filename);
	if(!directory){
		if( mkdir(filename,0755) ){
			perror("Could not create directory");
			exit(1);
		}
	}else{
		closedir(directory);
	}

	while(!feof(g.file.handle)){
		fscanf(g.file.handle,"%i\n",&window);
		XImage *thumbnail=thumbnail_generate(window);

		sprintf(filename, "%s/.rpexpose/%i", g.file.home, window);
		thumbnail_write(thumbnail,filename);
		XDestroyImage(thumbnail);
	}

	XCloseDisplay(g.x.display);
	return 0;
}

int rpexpose_select(){
	g.status=S_STARTUP;
	g.x.display = XOpenDisplay(getenv("DISPLAY"));

	load_rcdefaults();
	load_rcfile();
	load_input();
	load_xwindow();
	load_buffer();

	atexit(clean_up);

	// And here is when the magic is supposed to happen...
	
	XMapWindow(g.x.display,g.x.window);

	g.status=S_RUNNING;

	XEvent e;
	for(;;){
		XNextEvent(g.x.display, &e);
		switch(e.type){
		case MapNotify:
			XGrabKeyboard(g.x.display,g.x.window,False,GrabModeSync,GrabModeAsync,CurrentTime);
			break;
		case KeyPress:{
			KeySym keysym=XKeycodeToKeysym(g.x.display,e.xkey.keycode,0);
			if( keysym<256 && isdigit(keysym) ){
				if( g.p.selected->children[keysym-'0'] ){
					g.p.selected=g.p.selected->children[keysym-'0'];
					event_move(g.p.selected->window);
				}
				break;
			}
			KeyCode keycode=XKeysymToKeycode(g.x.display,keysym);
			parse_command(g.rc.keybindings[keycode]);
			break;
		}
		case Expose:
			event_redraw(e.xexpose.x,e.xexpose.y,e.xexpose.width,e.xexpose.height);
			event_move(g.gui.selected);
		}
	}
	return 0;
}


void clean_up(){
	g.status=S_SHUTDOWN;

	XUngrabKeyboard(g.x.display,CurrentTime);
	XUnmapWindow(g.x.display,g.x.window);
	
	{
		int i;
		for(i=0; i<256; ++i) free(g.rc.keybindings[i]);
	}

	{
		thumbnail_t *i=g.gui.thumbs, *prev;
		i->right->left=NULL;
		while(i){
			free(i->name);
			free(i->id);
			XDestroyImage(i->image);
			prev=i;
			i=i->left;
			free(prev);
		}
	}

	XFreeGC(g.x.display,g.x.gc);
	XFreeGC(g.x.display,g.x.rgc);
	
	XFreePixmap(g.x.display,g.x.buffer);
	XCloseDisplay(g.x.display);
	return;
}

