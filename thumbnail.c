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

const int THUMB_WIDTH=128;
const int THUMB_HEIGHT=128;
const int THUMB_PADDING=32;
const int THUMB_OFFSET=0;

typedef union _pixel pixel_t;

union _pixel{
	struct{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	} d;
	unsigned int b;
};

inline long thumbnail_pixel(XImage *ximage, int x, int y, int scale){
	unsigned int r=0, g=0, b=0, a=0, d=0;
	unsigned int i,j;
	pixel_t p;
	x*=scale;
	y*=scale;
	for(i=x+scale; i>x; --i)
		for(j=y+scale; j>y; --j){
			p.b=XGetPixel(ximage, i, j);
			b+=p.d.b;
			g+=p.d.g;
			r+=p.d.r;
			a+=p.d.a;
		}
	d=scale*scale;
	p.d.b=b/d;
	p.d.g=g/d;
	p.d.r=r/d;
	p.d.a=a/d;
	return p.b;	
}

XImage *thumbnail_generate(Window window){
	unsigned int width, height, depth, border, offset, x, y, thumb_width, thumb_height;
	double scale;
	Window root;
	XGetGeometry(g.x.display, window, &root, &x, &y, &width, &height, &border, &depth);

	XImage *ximage=XGetImage(g.x.display, window, 0, 0, width, height, AllPlanes, ZPixmap);

	Visual *visual=XDefaultVisual(g.x.display,0);

	if(width <= THUMB_WIDTH && height <= THUMB_HEIGHT){
		return ximage;
	}else if( height > width ){
		scale=1.0*height/THUMB_HEIGHT;
		thumb_width=width/scale;
		thumb_height=THUMB_HEIGHT;
	}else{
		scale=1.0*width/THUMB_WIDTH;
		thumb_height=height/scale;
		thumb_width=THUMB_WIDTH;
	}

	XImage *thumbnail=XCreateImage(g.x.display, visual,
								   depth, ZPixmap, THUMB_OFFSET,
								   malloc(thumb_width*thumb_height*4),
								   thumb_width, thumb_height,
								   THUMB_PADDING, thumb_width*4);

	for(x=thumb_width-1; x; --x)
		for(y=thumb_height-1; y; --y)
			XPutPixel(thumbnail, x,y, thumbnail_pixel(ximage, x, y, scale));

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
	Visual *visual=XDefaultVisual(g.x.display,0);
	
	FILE *file=fopen(filename, "rb");
	if( !file ){
		char *data=malloc(THUMB_WIDTH*THUMB_HEIGHT*4);
		memset(data,-1,THUMB_WIDTH*THUMB_HEIGHT*4);
		return XCreateImage(g.x.display, visual,
							 	  XDefaultDepth(g.x.display,0), ZPixmap,
								  THUMB_OFFSET, data,
								  THUMB_WIDTH, THUMB_HEIGHT,
								  THUMB_PADDING, 4*THUMB_WIDTH);
	}

	header_t h;
	fread(&h,sizeof(header_t),1,file);
	
	int size=h.width*h.height*4;
	char *data=malloc(size);

	fread(data,sizeof(char),size,file);

	fclose(file);
	
	return XCreateImage(g.x.display, visual,
					    h.depth, ZPixmap,
						THUMB_OFFSET, data,
						h.width, h.height,
						THUMB_PADDING, h.width*4);
}

