#include "rpexpose.h"

#define MIN_WIDTH 200

int colon_exit(){
	XFillRectangle(g.x.display, g.x.window, g.x.rgc, 
						g.colon.x, g.colon.y, 
						g.colon.width, g.colon.height);

	g.status=S_SELECT;
	g.colon.length=0;
	g.colon.buffer[0]='\0';
}

int colon_init(){
	g.colon.font=XQueryFont(g.x.display,XGContextFromGC(g.x.gc));
	g.status=S_COLON;
	g.colon.length=0;
	g.colon.buffer[0]='\0';
	
	g.colon.height=g.colon.font->max_bounds.ascent+2*g.rc.text_padding;

	g.colon.width=XTextWidth(g.colon.font,g.colon.buffer,g.colon.length)+2*g.rc.text_padding;
	g.colon.width=(g.colon.width>MIN_WIDTH)?(g.colon.width):(MIN_WIDTH);

	g.colon.x=g.x.width-g.colon.width;
	g.colon.y=g.x.height-g.colon.height;

}

int colon_refresh(char c){
	g.colon.buffer[g.colon.length]=c;
	g.colon.buffer[++g.colon.length]='\0';
	g.colon.cursor=g.colon.length;
	g.colon.height=g.colon.font->max_bounds.ascent+2*g.rc.text_padding;

	g.colon.width=XTextWidth(g.colon.font,g.colon.buffer,g.colon.length)+2*g.rc.text_padding;
	g.colon.width=(g.colon.width>MIN_WIDTH)?(g.colon.width):(MIN_WIDTH);

	g.colon.x=g.x.width-g.colon.width;
	g.colon.y=g.x.height-g.colon.height;
}

int colon_redraw(){
	XFillRectangle(g.x.display, g.x.window, g.x.rgc,
						g.colon.x, g.colon.y, 
						g.colon.width, g.colon.height);
	
	XDrawString(g.x.display, g.x.window, g.x.gc,
					g.colon.x+g.rc.text_padding, g.colon.y+g.colon.height-g.rc.text_padding,
					g.colon.buffer, g.colon.length);

	XDrawRectangle(g.x.display, g.x.window, g.x.gc,
						g.colon.x, g.colon.y,
						g.colon.width, g.colon.height);
	return 0;
}

int colon_select(){
	char buffer[BUFFER_SIZE];
	char temp[SMALL_BUFFER_SIZE];
	char *i, *j=g.rc.colon_exec;

	*buffer='\0';

	for(;;){
		i=j;
		while( (*i)?(*i!='%'):(0) ) ++i;
		
		if(!*i){
			strcat(buffer,j);
			break;
		}
		
		*i='\0';
		strcat(buffer,j);
	
		j=1+(++i);
	
		switch(*i){
		case 's':
			strcat(buffer,g.colon.buffer);
			break;
		default:
			return 1;
		}
	}
	
	system(buffer);
	exit(0);
}
