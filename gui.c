#include "rpexpose.h"

int load_input(){
	// Parse the input file, and generate a linked list of thumbnails
	char buffer[BUFFER_SIZE];
	
	thumbnail_t *head=NULL, *i=NULL;

	g.gui.num_thumbs=0;
	g.gui.thumbs=NULL;

	for(;;){
		fgets(buffer,BUFFER_SIZE,g.file.handle);

		if( feof(g.file.handle) ) break;

		char *id, *p=buffer;
	
		// Does the list exist? create it!
		if(i){
			i->next=malloc(sizeof(thumbnail_t));
			i=i->next;
		} else {
			head=i=malloc(sizeof(thumbnail_t));
		}

		i->selected=atoi(p);
		while( *p++!=':' );
		
		i->xid=atoi(p);
		while( *p++!=':' );
		
		id=p;
		while( *p++!=':' );
		*(p-1)='\0';
		i->id=strdup(id);

		i->name=strdup(p);
		i->next=NULL;
		g.gui.num_thumbs++;
	}

	
	load_xwindow();

	// Transfer that linked list to an array (now that we know the length) for faster random access
	g.gui.thumbs=malloc(g.gui.num_thumbs*sizeof(thumbnail_t));
	g.gui.selected=-1;


	int j;
	thumbnail_t *prev=NULL;
	for(j=0, i=head; i; i=i->next, ++j){
		free(prev);
		sprintf(buffer,"%s/.rpexpose/%i",g.file.home,i->xid);

		// Read in the file
		if(i->selected && g.gui.selected==-1){
			XImage *ximage=thumbnail_generate(i->xid);
			g.gui.thumbs[j].image=ximage;
			g.gui.thumbs[j].width=ximage->width;
			g.gui.thumbs[j].height=ximage->height;
			thumbnail_write(ximage,buffer);
			g.gui.selected=j;
		}else{
			g.gui.thumbs[j].image=thumbnail_read(buffer);
			g.gui.thumbs[j].width=g.gui.thumbs[j].image->width;
			g.gui.thumbs[j].height=g.gui.thumbs[j].image->height;
		}

		// Get the X geometry
		g.gui.thumbs[j].xid=i->xid;
		g.gui.thumbs[j].id=i->id;
		g.gui.thumbs[j].name=i->name;
		prev=i;
	}
	free(prev);

	return 0;
}

int load_xwindow(){
	// Get the geometry of the window from the id of thumbnails
	float unit;
	if(g.rc.widescreen){
		unit=sqrt( 1.0 * g.gui.num_thumbs / (WIDE_WIDTH * WIDE_HEIGHT) );
		g.gui.width=WIDE_WIDTH*unit+1;
		g.gui.height=WIDE_HEIGHT*unit+1;
	} else {
		unit=sqrt( 1.0 * g.gui.num_thumbs / (NORMAL_WIDTH * NORMAL_HEIGHT) );
		g.gui.width=NORMAL_WIDTH*unit+1;
		g.gui.height=NORMAL_HEIGHT*unit+1;
	}
	g.gui.height -= (g.gui.width*g.gui.height - g.gui.num_thumbs)/g.gui.width;
	
	g.x.width=g.rc.thumb_padding+g.gui.width*(g.rc.thumb_padding+THUMB_WIDTH);
	g.x.height=g.rc.thumb_padding+g.gui.height*(g.rc.thumb_padding+THUMB_HEIGHT);


	// Create X Data structures
	g.x.window= XCreateSimpleWindow(g.x.display,
									XDefaultRootWindow(g.x.display),
									0,0,
									g.x.width,g.x.height,
									g.rc.border_padding,
									XBlackPixel(g.x.display,0),
									XWhitePixel(g.x.display,0));

	XStoreName(g.x.display,g.x.window,"RpExpose");

	g.x.gc = XCreateGC(g.x.display,g.x.window,0,NULL);
	XSetForeground(g.x.display,g.x.gc,XBlackPixel(g.x.display,0));
	XSetBackground(g.x.display,g.x.gc,XWhitePixel(g.x.display,0));

	g.x.rgc = XCreateGC(g.x.display,g.x.window,0,NULL);
	XSetForeground(g.x.display,g.x.rgc,XWhitePixel(g.x.display,0));
	XSetBackground(g.x.display,g.x.rgc,XBlackPixel(g.x.display,0));

	XSelectInput(g.x.display,g.x.window, StructureNotifyMask | KeyPressMask | ExposureMask);

	XSetTransientForHint(g.x.display, g.x.window, XDefaultRootWindow(g.x.display));

	g.x.buffer = XCreatePixmap(g.x.display, g.x.window, g.x.width, g.x.height, XDefaultDepth(g.x.display,0));

	XFillRectangle(g.x.display, g.x.buffer, g.x.rgc, 0, 0, g.x.width, g.x.height);
	
	return 0;
}

inline int draw_text(Drawable d, char *text, int x, int y){
	int len=strlen(text);
	XFontStruct *font=XQueryFont(g.x.display,XGContextFromGC(g.x.gc));
	
	int height=font->max_bounds.ascent+2*g.rc.text_padding;

	int width=XTextWidth(font,text,len)+2*g.rc.text_padding;

	XFillRectangle(g.x.display, g.x.buffer, g.x.rgc, x, y, width, height);

	XDrawString(g.x.display, g.x.buffer, g.x.gc, x+g.rc.text_padding, y+height-g.rc.text_padding, text, len);
	
	XDrawRectangle(g.x.display, g.x.buffer, g.x.gc, x, y, width, height);
	
}

inline int draw_thumbnail(Drawable d, thumbnail_t *t){
	XPutImage(g.x.display, d, g.x.gc, t->image, 0,0, t->x, t->y, t->width, t->height);

	XDrawRectangle(g.x.display, g.x.buffer, g.x.gc, t->x, t->y, t->width, t->height);

}

int load_buffer(){
	int x, y, i;
	for(y=0; y<g.gui.height-1; ++y){
		for(x=0; x<g.gui.width; ++x){
			int i=g.gui.width*y+x;
			thumbnail_t *t=&g.gui.thumbs[i];

			t->width=g.gui.thumbs[i].width;
			t->height=g.gui.thumbs[i].height;

			t->x=g.rc.thumb_padding+x*(g.rc.thumb_padding+THUMB_WIDTH)+(THUMB_WIDTH-t->width)/2;
			t->y=g.rc.thumb_padding+y*(g.rc.thumb_padding+THUMB_HEIGHT)+(THUMB_HEIGHT-t->height)/2;

			t->right=(i+1)%g.gui.num_thumbs;
			t->left=(i+g.gui.num_thumbs-1)%g.gui.num_thumbs;

			// Draw it already, damn it!
			draw_thumbnail(g.x.buffer,t);
			draw_text(g.x.buffer, g.gui.thumbs[i].id, t->x, t->y);
			}
	}

	// Populate the final row
	int left_over=g.gui.num_thumbs-g.gui.width*(g.gui.height-1);
	int offset=(g.gui.width-left_over)*(g.rc.thumb_padding+THUMB_WIDTH)/2;
	
	for(x=0; x<left_over; ++x){
		int i=g.gui.width*y+x;
		thumbnail_t *t=&g.gui.thumbs[i];

		t->width=g.gui.thumbs[i].width;
		t->height=g.gui.thumbs[i].height;

		t->x=g.rc.thumb_padding+x*(g.rc.thumb_padding+THUMB_WIDTH)+(THUMB_WIDTH-t->width)/2+offset;
		t->y=g.rc.thumb_padding+y*(g.rc.thumb_padding+THUMB_HEIGHT)+(THUMB_HEIGHT-t->height)/2;

		t->right=(i+1)%g.gui.num_thumbs;
		t->left=(i+g.gui.num_thumbs-1)%g.gui.num_thumbs;

		// Draw it already, damn it!
		draw_thumbnail(g.x.buffer,t);
		draw_text(g.x.buffer, g.gui.thumbs[i].id, t->x, t->y);
	}
	return 0;
}

int event_draw(){
	XCopyArea(g.x.display,
			  g.x.buffer,
			  g.x.window,
			  g.x.gc,
			  0,0,
			  g.x.width,g.x.height,
			  0,0);
	
	return 0;
}

int event_redraw(int x, int y, int width, int height){
	XCopyArea(g.x.display,
			  g.x.buffer,
			  g.x.window,
			  g.x.gc,
			  x,y,
			  width,height,
			  x,y);
	
	return 0;
}

int event_move(int new){
	thumbnail_t o=g.gui.thumbs[g.gui.selected];
	int pad=g.rc.border_padding, dpad=2*pad;


	XSetLineAttributes(g.x.display,g.x.rgc,g.rc.border_padding,LineSolid,CapNotLast,JoinMiter);
	XSetLineAttributes(g.x.display,g.x.gc,g.rc.border_padding,LineSolid,CapNotLast,JoinMiter);

	XDrawRectangle(g.x.display,
				   g.x.window,
				   g.x.rgc,
				   o.x-pad, o.y-pad,
				   o.width+dpad, o.height+dpad);

	thumbnail_t n=g.gui.thumbs[new];
	XDrawRectangle(g.x.display,
				   g.x.window,
				   g.x.gc,
				   n.x-pad, n.y-pad,
				   n.width+dpad, n.height+dpad);
	
	XSetLineAttributes(g.x.display,g.x.rgc,1,LineSolid,CapNotLast,JoinMiter);
	XSetLineAttributes(g.x.display,g.x.gc,1,LineSolid,CapNotLast,JoinMiter);

	g.gui.selected = new;
}

int event_select(){
	char buffer[BUFFER_SIZE];
	char temp[SMALL_BUFFER_SIZE];
	char *i, *j=g.rc.select_exec;

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
		case 'x':
			sprintf(temp,"%i",g.gui.thumbs[g.gui.selected].xid);
			strcat(buffer,temp);
			break;
		case 'n':
			strcat(buffer,g.gui.thumbs[g.gui.selected].name);
			break;
		case 'i':
			sprintf(temp,"%s",g.gui.thumbs[g.gui.selected].id);
			strcat(buffer,temp);
			break;
		default:
			return 1;
		}
	}

	system(buffer);
	exit(0);
	return 0;
}

