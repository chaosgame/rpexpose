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

const int BUFFER_SIZE=1024;
const int SMALL_BUFFER_SIZE=128;
const int ARGV_SIZE=64;

char *parse_split(char *command){
	// Read until a space is found
	char *arguments=command;
	while( (*arguments)?(!isspace(*arguments)):(0) ) ++arguments;

	// Isolate the command
	*arguments='\0';

	// If the command has length 0
	if(arguments==command)
		return NULL;
	++arguments;

	// If the end of the string has been reached
	if( !*arguments )
		return NULL;

	// Read until a non whitespace character is found
	while((*arguments)?(isspace(*arguments)):(0)) ++arguments;

	if( !*arguments )
		return NULL;

	return arguments;
}

int load_rcdefaults(){
	int i;
	for(i=0; i<256; ++i)
		g.rc.keybindings[i]=NULL;
	for(i=0; i<10; ++i)
			g.p.top.children[i]=NULL;

	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_h)]=strdup("left");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_k)]=strdup("left");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_j)]=strdup("right");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_l)]=strdup("right");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_q)]=strdup("quit");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_space)]=strdup("select");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Escape)]=strdup("escape");

	g.rc.select_exec=strdup("ratpoison -c \"select %i\"");

	g.rc.text_padding=2;
	g.rc.border_padding=4;
	g.rc.thumb_padding=8;
	g.rc.widescreen=0;
}

int load_rcfile(){
	char buffer[BUFFER_SIZE];
	int error=0;


	sprintf(buffer,"%s/.rpexposerc",g.file.home);
	FILE *file=fopen(buffer,"r");

	if(!file){
		strcpy(buffer,"/etc/rpexposerc");
		file=fopen(buffer,"r");
		if(!file)
			return 0;
	}

	while(!feof(file)){
		*buffer='\0';
		fgets(buffer,BUFFER_SIZE,file);
		error+=parse_command(buffer);
	}
	return 0;
}

int parse_command(char *command){
	if(!command) return 0;
	if(*command=='#') return 0;

	char *arguments=parse_split(command);

	if(*command=='\0') return 0;
	
	switch(g.status){
	case S_RUNNING:
		if( !strcmp(command,"escape") ){
			g.p.selected=&g.p.top;
			return 0;
		}
		if( !strcmp(command,"quit") )
			exit(0);
		if( !strcmp(command,"left") )
			return event_move(g.gui.selected->left);
		if( !strcmp(command,"right") )
			return event_move(g.gui.selected->right);
		if( !strcmp(command,"select") )
			return event_select();
	case S_STARTUP:
		if( !strcmp(command,"set") )
			return parse_set(arguments);
		if( !strcmp(command,"bind") )
			return parse_bind(arguments,0);
		if( !strcmp(command,"unbind") )
			return parse_bind(arguments,1);
		if( !strcmp(command,"exec") )
			return system(command);
	case S_SHUTDOWN:
	default:
			error("Bad Command in rc file");
			return 1;
	}
}

int parse_set(char *command){
	char *arguments=parse_split(command);

	if(!arguments){
		error("Set called with too few arguments in rc file");
		return 1;
	}

	if( !strcmp(command,"widescreen") ){
		int value = parse_truth(arguments);
		if( value == -1 )
			return 1;
		
		g.rc.widescreen=value;
		return 0;
	}
	if( !strcmp(command,"thumb_padding") ){
		g.rc.thumb_padding=atoi(arguments);
		return 0;
	}
	if( !strcmp(command,"text_padding") ){
		g.rc.text_padding=atoi(arguments);
		return 0;
	}
	if( !strcmp(command,"border_padding") ){
		g.rc.border_padding=atoi(arguments);
		return 0;
	}
	if( !strcmp(command,"select_exec") ){
		free(g.rc.select_exec);
		g.rc.select_exec=strdup(arguments);
		return 0;
	}

	error("Set called with unknown argument in rc file");
	return 1;
}

int parse_bind(char *command, int unbind){
	char *arguments=parse_split(command);
	
	if(!arguments){
		error("Bind called with too few options in rc file");
		return 1;
	}

	KeySym keysym=XStringToKeysym(command);
	KeyCode keycode=XKeysymToKeycode(g.x.display, keysym);

	free(g.rc.keybindings[keycode]);
	g.rc.keybindings[keycode]=(unbind)?(NULL):(strdup(arguments));

	return 0;
}

int parse_truth(char *arguments){
	if(parse_split(arguments)){
		error("Unknown boolean value in rc file");
		return -1;
	}
	if( !strcmp(arguments,"true") )
		return 1;
	if( !strcmp(arguments,"t") )
		return 1;
	if( !strcmp(arguments,"1") )
		return 1;
	if( !strcmp(arguments,"false") )
		return 0;
	if( !strcmp(arguments,"f") )
		return 0;
	if( !strcmp(arguments,"0") )
		return 0;
	
	error("Unknown boolean value in rc file");
	return -1;
}

