#include "rpexpose.h"

const char *argp_program_version = "rpexpose 0.1.0";

const char *argp_program_bug_address = "<nlawren2@uiuc.edu>";

static char doc[] = "";

static char args_doc[] = "";

static struct argp_option cmdline_options[] = {
	{"clean",		'c', NULL,		0, "Cleans the /var/run/rpexpose directory" },
	{"generate",	'g', "FILE",	0, "Generates ximage dumps from a list of top level windows"},
	{"delete",		'd', "FILE",	0, "Deletes all ximage dumps in a list of top level windows"},
	{"select",		's', "FILE",	0, "Creates a selection dialog for a list of managed windows"},
	{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state){
	switch(key){
	case 'c':
		g.action=CLEAR;
		break;
	case 'g':
		g.action=GENERATE;
		g.file.name=arg;
		break;
	case 'd':
		g.action=DELETE;
		g.file.name=arg;
		break;
	case 's':
		g.action=SELECT;
		g.file.name=arg;
		break;
	case ARGP_KEY_END:
		if( g.action ) break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { cmdline_options, parse_opt, args_doc, doc };

int main(int argc, char **argv){
	g.action = UNKNOWN;
	g.file.name = NULL;
	g.file.home = getenv("HOME");

	argp_parse(&argp, argc, argv, 0, 0, NULL); 
	
	if( g.file.name == NULL )
		g.file.handle = NULL;
	else if( strcmp(g.file.name,"-") )
		g.file.handle=fopen(g.file.name,"r");
	else
		g.file.handle=stdin;

	int error=1;
	switch(g.action){
	case CLEAR:
		error=rpexpose_clean();
		break;
	case GENERATE:
		error=rpexpose_generate();
		break;
	case DELETE:
		error=rpexpose_delete();
		break;
	case SELECT:
		error=rpexpose_select();
		break;
	}

	if( g.file.handle )
		fclose(g.file.handle);


	return error;
}

int rpexpose_clean(){
	struct dirent *ep;
	
	char filename[BUFFER_SIZE];
	sprintf(filename,"%s/.rpexpose",g.file.home);

	DIR *directory = opendir(filename);

	if(directory){
		while( ep = readdir(directory) ){
			sprintf(filename,"%s/.rpexpose/%s",g.file.home,ep->d_name);
			remove(filename);
		}
		closedir(directory);
	}
	else{
		perror("Couldn't open $HOME/.rpexpose/\n");
		return 1;
	}
	return 0;
}

int rpexpose_generate(){
	g.x.display = XOpenDisplay(getenv("DISPLAY"));

	Window window;
	
	char filename[BUFFER_SIZE];

	while(!feof(g.file.handle)){
		fscanf(g.file.handle,"%i\n",&window);
		XImage *thumbnail=generate_thumbnail(window);

		sprintf(filename, "%s/.rpexpose/%i.xpm", g.file.home, window);
		XpmWriteFileFromImage(g.x.display, filename, thumbnail, NULL, NULL);
	}

	XCloseDisplay(g.x.display);
	return 0;
}

int rpexpose_delete(){
	char window[SMALL_BUFFER_SIZE];
	char filename[BUFFER_SIZE];

	while(!feof(g.file.handle)){
		fscanf(g.file.handle,"%s\n",&window);
		sprintf(filename,"%s/.rpexpose/%s.xpm",g.file.home,window);
		
		int error = remove(filename);
	}
	return 0;
}

int rpexpose_select(){
	g.x.display = XOpenDisplay(getenv("DISPLAY"));

	load_rcdefaults();
	load_rcfile();
	load_input();
	load_pixmap();


	// And here is when the magic is supposed to happen...
	
	XMapWindow(g.x.display,g.x.window);

	XEvent e;
	for(;;){
		XNextEvent(g.x.display, &e);
		switch(e.type){
		case MapNotify:
			event_redraw();
			event_move(g.gui.selected);
			break;
		case KeyPress:{
			char *i, *command=g.rc.keybindings[e.xkey.keycode];

			if(!command) break;

			// Chomp chomp
			for(i=command; (*i)?(*i!='\n'):(0) ; ++i);
			*i='\0';

			if( !strcmp(command,"quit") )
				goto Quit;
			else if( !strcmp(command,"left") )
					event_move(g.gui.thumbs[g.gui.selected].left);
			else if( !strcmp(command,"right") )
					event_move(g.gui.thumbs[g.gui.selected].right);
			else if( !strcmp(command,"up") )
					event_move(g.gui.thumbs[g.gui.selected].left);
			else if( !strcmp(command,"down") )
					event_move(g.gui.thumbs[g.gui.selected].right);
			else if( !strcmp(command,"select") ){
				event_select();
				goto Quit;
			}else
				parse_command(command);
		break;
		}
		}
	}

Quit:

	// Mop, Mop
	clean_up();
	XCloseDisplay(g.x.display);
}


int clean_up(){
	int i;
	for(i=0; i<256; ++i) free(g.rc.keybindings[i]);

	for(i=0; i<g.gui.num_thumbs; ++i){
		free(g.gui.thumbs[i].name);
		XFreePixmap(g.x.display,g.gui.thumbs[i].pixmap);
	}

	free(g.gui.thumbs);

	hooklist_t *prev=NULL;
	while(g.rc.exit){
		free(g.rc.exit->command);
		prev=g.rc.exit;
		g.rc.exit=g.rc.exit->next;
		free(g.rc.exit);
	}

	XFreePixmap(g.x.display,g.x.buffer);

	return 0;
}

