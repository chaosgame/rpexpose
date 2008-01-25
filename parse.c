#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpexpose.h"

int split_command(char *command, char **arguments){
	// Read until a space is found
	char *i=command;
	while( (*i)?(!isspace(*i)):(0) ) ++i;


	// Isolate the command
	*i='\0';

	// If the command has length 0
	if(i==command){
		*arguments=NULL;
		return 1;
	}
	++i;

	// If the end of the string has been reached
	if( !*i ){
		*arguments=NULL;
		return 0;
	}

	// Read until a non whitespace character is found
	while((*i)?(isspace(*i)):(0)) ++i;

	if( !*i ){
		*arguments=NULL;
		return 0;
	}

	*arguments=i;
	return 0;
}

int load_rcdefaults(){
	int i;
	for(i=0; i<256; ++i)
		g.rc.keybindings[i]=NULL;

	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Left)]=strdup("left");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Up)]=strdup("up");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Down)]=strdup("down");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Right)]=strdup("right");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Return)]=strdup("select");
	g.rc.keybindings[XKeysymToKeycode(g.x.display, XK_Escape)]=strdup("quit");

	g.rc.select_exec=strdup("ratpoison -c \"select %i\"");

	g.rc.border_width=2;
	g.rc.widescreen=0;
}

int load_rcfile(){
	char buffer[BUFFER_SIZE];
	int error;


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
		if( error = parse_command(buffer) ){
			return error;
		}
	}
	return 0;
}

int parse_command(char *command){
	if(*command=='#') return 0;

	char *arguments;

	if(split_command(command,&arguments)) return 0;
	
	if( !strcmp(command,"set") )
		return parse_set(arguments);
	else if( !strcmp(command,"bind") )
		return parse_bind(arguments,0);
	else if( !strcmp(command,"unbind") )
		return parse_bind(arguments,1);
	else if( !strcmp(command,"exec") )
		return parse_exec(arguments);
	else{
		fprintf(stderr,"Command not recognized");
		return 1;
	}
}

int parse_set(char *command){
	char *arguments;

	split_command(command,&arguments);

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
	}else if( !strcmp(command,"border_width") ){
		g.rc.border_width=atoi(arguments);
		return 0;
	}else if( !strcmp(command,"select_exec") ){
		free(g.rc.select_exec);
		g.rc.select_exec=strdup(arguments);
	}else{
		perror("Unknown variable set in rc file");
		return 1;
	}
}

int parse_exec(char *command){
	return system(command);
}

int parse_bind(char *command, int unbind){
	char *arguments;

	split_command(command,&arguments);

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
	char *trash;
	split_command(arguments,&trash);
	
	if(trash){
		perror("Unknown boolean value in rc file");
		return -1;
	}
	else if( !strcmp(arguments,"true") )
		return 1;
	else if( !strcmp(arguments,"t") )
		return 1;
	else if( !strcmp(arguments,"1") )
		return 1;
	else if( !strcmp(arguments,"false") )
		return 0;
	else if( !strcmp(arguments,"f") )
		return 0;
	else if( !strcmp(arguments,"0") )
		return 0;
	else{
		perror("Unknown boolean value in rc file");
		return -1;
	}
}

