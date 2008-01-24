#include "rpexpose.h"

XImage *generate_thumbnail(Window window){
	unsigned int width, height, depth, border, offset, scale, x, y, thumb_width, thumb_height;
	Window root;
	XGetGeometry(g.x.display, window, &root, &x, &y, &width, &height, &border, &depth);

	XImage *ximage=XGetImage(g.x.display, window, 0, 0, width, height, AllPlanes, ZPixmap);

	Visual *visual=XDefaultVisual(g.x.display,0);
	char *thumbnail_data;

	if( height > width ){
		scale=height/THUMB_HEIGHT;
		thumb_width=width/scale;
		thumb_height=THUMB_HEIGHT;
	}else{
		scale=width/THUMB_WIDTH;
		thumb_height=height/scale;
		thumb_width=THUMB_WIDTH;
	}

	thumbnail_data=malloc(thumb_width*thumb_height*4);

	int center=scale/2;
	for(x=thumb_width-1; x; --x)
		for(y=thumb_height-1; y; --y)
			SET_PIXEL(thumbnail_data, x,y,
					  XGetPixel(ximage, 
								scale*x+center,
								scale*y+center));

	XDestroyImage(ximage);
	
	return XCreateImage(g.x.display, 
			    visual, 
			    depth,
	 		    ZPixmap,
			    THUMB_OFFSET,
			    thumbnail_data,
			    thumb_width, thumb_height,
			    THUMB_PADDING,
			    thumb_width*4);
}

/*
XImage *generate_thumbnail(Window window){
	unsigned int width, height, depth, border, offset, scale, x, y, pixel;
	Window root;

	XGetGeometry(g.x.display, window, &root, &x, &y, &width, &height, &border, &depth);
	Pixmap pixmap = XCreatePixmap(g.x.display, root, width, height, depth);
	GC gc=XDefaultGC(g.x.display,0);


	XCopyArea(g.x.display, window, pixmap, gc, 0,0, width, height, 0, 0);

	// Code here

	return NULL;
}
*/
