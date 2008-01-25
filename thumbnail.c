#include "rpexpose.h"

XImage *thumbnail_generate(Window window){
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

	XImage *thumbnail=XCreateImage(g.x.display, visual,
								   depth, ZPixmap, THUMB_OFFSET,
								   malloc(thumb_width*thumb_height*4),
								   thumb_width, thumb_height,
								   THUMB_PADDING, thumb_width*4);

	int center=scale/2;
	for(x=thumb_width-1; x; --x)
		for(y=thumb_height-1; y; --y)
			XPutPixel(thumbnail, x,y, XGetPixel(ximage, scale*x+center, scale*y+center));

	XDestroyImage(ximage);
	
	return thumbnail;
}

typedef struct _header header_t;
struct _header{
	int depth;
	int width;
	int height;
};

int thumbnail_write(XImage *thumbnail, char *filename){
	FILE *file=fopen(filename, "wb");

	header_t h;
	h.width=thumbnail->width;
	h.height=thumbnail->height;
	h.depth=thumbnail->depth;
	
	int size=h.width*h.height*4;
	fwrite(&h,sizeof(header_t),1,file);
	fwrite(thumbnail->data,sizeof(char),size,file);
	
	fclose(file);
}

XImage *thumbnail_read(char *filename){
	FILE *file=fopen(filename, "rb");
	header_t h;
	fread(&h,sizeof(header_t),1,file);
	
	int size=h.width*h.height*4;
	char *data=malloc(size);

	fread(data,sizeof(char),size,file);

	fclose(file);
	
	Visual *visual=XDefaultVisual(g.x.display,0);

	return XCreateImage(g.x.display, visual,
					    h.depth, ZPixmap,
						THUMB_OFFSET, data,
						h.width, h.height,
						THUMB_PADDING, h.width*4);
}

