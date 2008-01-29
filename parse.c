#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpexpose.h"

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

	g.p.top.children=NULL;

	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Left)]=strdup("left");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Up)]=strdup("up");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Down)]=strdup("down");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Right)]=strdup("right");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Return)]=strdup("select");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Escape)]=strdup("quit");

	g.rc.colon_exec=strdup("ratpoison -c \"select %s\"");
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
	case S_INSERT:
//		if( !strcmp(command,"escape") )
//			return insert_exit();
		if( !strcmp(command,"select") )
			return event_select();
	case S_RUNNING:
		if( !strcmp(command,"escape") )
			exit(0);
		if( !strcmp(command,"quit") )
			exit(0);
		if( !strcmp(command,"left") )
			return event_move(g.gui.thumbs[g.gui.selected].left);
		if( !strcmp(command,"right") )
			return event_move(g.gui.thumbs[g.gui.selected].right);
		if( !strcmp(command,"up") )
			return event_move(g.gui.thumbs[g.gui.selected].left);
		if( !strcmp(command,"down") )
			return event_move(g.gui.thumbs[g.gui.selected].right);
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
			fprintf(stderr,"Bad Command\n");
			return 1;
	}
}

int parse_set(char *command){
	char *arguments=parse_split(command);

	if(!arguments){
		perror("set called with no arguments in rc file");
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

	fprintf(stderr,"Unknown variable\n");
	return 1;
}

int parse_bind(char *command, int unbind){
	char *arguments=parse_split(command);
	
	if(!arguments){
		perror("bind called with no options in rc file");
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
		perror("Unknown boolean value in rc file");
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
	
	perror("Unknown boolean value in rc file");
	return -1;
}

