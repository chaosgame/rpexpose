#include "rpexpose.h"

int load_input(){
	// Parse the input file, and generate a linked list of thumbnails
	int xid, number;
	char buffer[BUFFER_SIZE];
	
	thumbnail_t *head=NULL, *i=NULL;

	g.gui.num_thumbs=0;
	g.gui.thumbs=NULL;

	for(;;){
		fgets(buffer,BUFFER_SIZE,g.file.handle);

		if( feof(g.file.handle) ) break;

		char *p=buffer;
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
		
		i->number=atoi(p);
		while( *p++!=':' );


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

		// Read in the file
		sprintf(buffer,"%s/.rpexpose/%i",g.file.home,i->xid);
		g.gui.thumbs[j].image=thumbnail_read(buffer);
		g.gui.thumbs[j].width=g.gui.thumbs[j].image->width;
		g.gui.thumbs[j].height=g.gui.thumbs[j].image->height;

		if(i->selected && g.gui.selected==-1)
			g.gui.selected=j;

		// Get the X geometry
		g.gui.thumbs[j].xid=i->xid;
		g.gui.thumbs[j].number=i->number;
		g.gui.thumbs[j].name=i->name;
		prev=i;
	}
	free(prev);

	return 0;
}

int load_xwindow(){
	// Get the geometry of the window from the number of thumbnails
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
	
	g.x.width=g.rc.border_width+g.gui.width*(g.rc.border_width+THUMB_WIDTH);
	g.x.height=g.rc.border_width+g.gui.height*(g.rc.border_width+THUMB_HEIGHT);


	// Create X Data structures
	g.x.window= XCreateSimpleWindow(g.x.display,
									XDefaultRootWindow(g.x.display),
									0,0,
									g.x.width,g.x.height,
									g.rc.border_width,
									XBlackPixel(g.x.display,0),
									XWhitePixel(g.x.display,0));

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

int load_pixmap(){
	int x, y, i;
	for(y=0; y<g.gui.height-1; ++y){
		for(x=0; x<g.gui.width; ++x){
			int i=g.gui.width*y+x;
			unsigned int x_pos, y_pos, width=g.gui.thumbs[i].width, height=g.gui.thumbs[i].height;
			
			x_pos=g.rc.border_width+x*(g.rc.border_width+THUMB_WIDTH)+(THUMB_WIDTH-width)/2;
			y_pos=g.rc.border_width+y*(g.rc.border_width+THUMB_HEIGHT)+(THUMB_HEIGHT-height)/2;

			g.gui.thumbs[i].right=(i+1)%g.gui.num_thumbs;
			g.gui.thumbs[i].left=(i+g.gui.num_thumbs-1)%g.gui.num_thumbs;

			g.gui.thumbs[i].x=x_pos;
			g.gui.thumbs[i].y=y_pos;
			g.gui.thumbs[i].width=width;
			g.gui.thumbs[i].height=height;

			// Draw it already, damn it!
			XPutImage(g.x.display,
					  g.x.buffer,
					  g.x.gc,
					  g.gui.thumbs[i].image,
					  0,0,
					  x_pos, y_pos,
					  width,height);

			XDrawRectangle(g.x.display,
						   g.x.buffer,
						   g.x.gc,
						   x_pos, y_pos,
						   width, height);
		}
	}

	// Populate the final row
	int left_over=g.gui.num_thumbs-g.gui.width*(g.gui.height-1);
	int offset=(g.gui.width-left_over)*(g.rc.border_width+THUMB_WIDTH)/2;
	
	for(x=0; x<left_over; ++x){
		int i=g.gui.width*y+x;
		unsigned int x_pos, y_pos, width=g.gui.thumbs[i].width, height=g.gui.thumbs[i].height;
		
		x_pos=g.rc.border_width+x*(g.rc.border_width+THUMB_WIDTH)+(THUMB_WIDTH-width)/2+offset;
		y_pos=g.rc.border_width+y*(g.rc.border_width+THUMB_HEIGHT)+(THUMB_HEIGHT-height)/2;

		g.gui.thumbs[i].right=(i+1)%g.gui.num_thumbs;
		g.gui.thumbs[i].left=(i+g.gui.num_thumbs-1)%g.gui.num_thumbs;

		g.gui.thumbs[i].x=x_pos;
		g.gui.thumbs[i].y=y_pos;
		g.gui.thumbs[i].width=width;
		g.gui.thumbs[i].height=height;

		// Draw it already, damn it!
		XPutImage(g.x.display,
				  g.x.buffer,
				  g.x.gc,
				  g.gui.thumbs[i].image,
				  0,0,
				  x_pos, y_pos,
				  width,height);

		XDrawRectangle(g.x.display,
					   g.x.buffer,
					   g.x.gc,
					   x_pos, y_pos,
					   width, height);
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
	XDrawRectangle(g.x.display,
				   g.x.window,
				   g.x.rgc,
				   o.x-2, o.y-2,
				   o.width+4, o.height+4);

	thumbnail_t n=g.gui.thumbs[new];
	XDrawRectangle(g.x.display,
				   g.x.window,
				   g.x.gc,
				   n.x-2, n.y-2,
				   n.width+4, n.height+4);
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
			sprintf(temp,"%i",g.gui.thumbs[g.gui.selected].number);
			strcat(buffer,temp);
			break;
		default:
			return 1;
		}
	}

	system(buffer);
	return 0;
}

