/* drawing.c */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "fb.h"

#include "structs.h"

void blit(SDL_Surface *des, SDL_Surface *src, SDL_Rect *srcrect, SDL_Rect *desrect) {
    blit_info info;

    info.dst = des->pixels;
    if (desrect != NULL) {
        info.dst_x = desrect->x;
        info.dst_y = desrect->y;
        info.dst_w = desrect->w;
        info.dst_h = desrect->h;
    }
    else {
        info.dst_x = 0;
        info.dst_y = 0;
        info.dst_w = des->w;
        info.dst_h = des->h;
    }
    info.dst_pitch = des->pitch;
    info.src = src->pixels;
    if (srcrect != NULL) {
        info.src_x = srcrect->x;
        info.src_y = srcrect->y;
    }
    else {
        info.src_x = 0;
        info.src_y = 0;
    }
    info.src_pitch = src->pitch;

    if (info.dst_x < 0) {
        info.dst_w += info.dst_x;
        if (info.dst_w <= 0)
            return;
        info.src_x -= info.dst_x;
        info.dst_x = 0;
    }
    if ((info.dst_x + info.dst_w) >= des->w) {
        info.dst_w = des->w - info.dst_x;
        if (info.dst_w <= 0)
            return;
    }
    if (info.dst_y < 0) {
        info.dst_h += info.dst_y;
        if (info.dst_h <= 0)
            return;
        info.src_y -= info.dst_y;
        info.dst_y = 0;
    }
    if ((info.dst_y + info.dst_h) >= des->h) {
        info.dst_h = des->h - info.dst_y;
        if (info.dst_h <= 0)
            return;
    }

    fb_blit(&info);
}

void blit_colorkey(SDL_Surface *des, SDL_Surface *src, SDL_Rect *srcrect, SDL_Rect *desrect) {
    blit_info info;

    info.dst = des->pixels;
    if (desrect != NULL) {
        info.dst_x = desrect->x;
        info.dst_y = desrect->y;
        info.dst_w = desrect->w;
        info.dst_h = desrect->h;
    }
    else {
        info.dst_x = 0;
        info.dst_y = 0;
        info.dst_w = des->w;
        info.dst_h = des->h;
    }
    info.dst_pitch = des->pitch;
    info.src = src->pixels;
    if (srcrect != NULL) {
        info.src_x = srcrect->x;
        info.src_y = srcrect->y;
    }
    else {
        info.src_x = 0;
        info.src_y = 0;
    }
    info.src_pitch = src->pitch;

    if (info.dst_x < 0) {
        info.dst_w += info.dst_x;
        if (info.dst_w <= 0)
            return;
        info.src_x -= info.dst_x;
        info.dst_x = 0;
    }
    if ((info.dst_x + info.dst_w) >= des->w) {
        info.dst_w = des->w - info.dst_x;
        if (info.dst_w <= 0)
            return;
    }
    if (info.dst_y < 0) {
        info.dst_h += info.dst_y;
        if (info.dst_h <= 0)
            return;
        info.src_y -= info.dst_y;
        info.dst_y = 0;
    }
    if ((info.dst_y + info.dst_h) >= des->h) {
        info.dst_h = des->h - info.dst_y;
        if (info.dst_h <= 0)
            return;
    }

    fb_blit_colorkey(&info, (pixel_t) 0xFF000000);
}

void drawscreen (SDL_Surface *renderer,uint stagedata[][22][32],SDL_Surface *tiles,uint room[],uint counter[],uint changeflag,Mix_Chunk *fx[],uint changetiles) {

	int coordx = 0;
	int coordy = 0;
	SDL_Rect srctiles = {0,0,8,8};
	SDL_Rect destiles = {0,0,8,8};
	uint data = 0;

	for (coordy=0; coordy<=21; coordy++) {
		for (coordx=0; coordx<=31; coordx++) {
			data = stagedata[room[0]][coordy][coordx];
			if ((data > 0) && (data != 99)) {
				destiles.x = coordx * 8;
				destiles.y = coordy * 8;
				if (data < 200) {
					srctiles.w = 8;
					srctiles.h = 8;
					if (data < 101) {
						srctiles.y = 0;
						if (data == 84) /* Cross brightness */
							srctiles.x = (data - 1) * 8 + (counter[0]/8 * 8);
						else
							srctiles.x = (data - 1) * 8;
					}
					else {
						if (data == 154) {
							srctiles.x=600 + ((counter[0] / 8) * 16);
							srctiles.y=0;
							srctiles.w=16;
							srctiles.h=24;
						}
						else {
							srctiles.y = 8;
							srctiles.x = (data - 101) * 8;
						}
					}
				}
				if ((data > 199) && (data < 300)) {
					srctiles.x = (data - 201) * 48;
					srctiles.y = 16;
					srctiles.w = 48;
					srctiles.h = 48;
				}
				if ((data > 299) && (data < 399)) {
					srctiles.x = 96 + ((data - 301) * 8);
					srctiles.y = 16;
					srctiles.w = 8;
					srctiles.h = 8;
					/* Door movement */
					if ((room[0] == 7) && ((counter[1] > 59) && (counter[1] < 71))) {
						if ((data == 347) || (data == 348) || (data == 349) || (data == 350)) {
							destiles.x += 2;
							if ((data == 350) && (counter[1] == 70))
								Mix_PlayChannel(-1, fx[3], 0); /* Sound of door */
						}
					}
				}
				/* Hearts */
				if ((data > 399) && (data < 405)) {
					srctiles.x = 96 + ((data - 401) * 8) + (32 * (counter[0] / 15));
					srctiles.y = 24;
					srctiles.w = 8;
					srctiles.h = 8;
				}
				/* Crosses */
				if ((data > 408) && (data < 429)) {
					srctiles.x = 96 + ((data - 401) * 8) + (32 * (counter[1] / 23));
					srctiles.y = 24;
					srctiles.w = 8;
					srctiles.h = 8;
				}

				if ((data > 499) && (data < 599)) {
					srctiles.x = 96 + ((data - 501) * 8);
					srctiles.y = 32;
					srctiles.w = 8;
					srctiles.h = 8;
				}
				if ((data > 599) && (data < 650)) {
					srctiles.x = 96 + ((data - 601) * 8);
					srctiles.y = 56;
					srctiles.w = 8;
					srctiles.h = 8;
				}
				if (data == 650) { /* Cup */
					srctiles.x = 584;
					srctiles.y = 87;
					srctiles.w = 16;
					srctiles.h = 16;
				}
				destiles.w = srctiles.w;
				destiles.h = srctiles.h;
				if ((data == 152) || (data == 137) || (data == 136)) {
					if (changeflag == 0) {
						srctiles.y = srctiles.y + (changetiles * 120);
						blit(renderer,tiles,&srctiles,&destiles);
					}
				}
				else {
					srctiles.y = srctiles.y + (changetiles * 120);
					blit(renderer,tiles,&srctiles,&destiles);
				}
			}
		}
	}

}

void statusbar (SDL_Surface *renderer,SDL_Surface *tiles,int room[],int lifes,int crosses,SDL_Surface *fonts,uint changetiles) {

	SDL_Rect srcbar = {448,104,13,12};
	SDL_Rect desbar = {0,177,13,12};
	SDL_Rect srcnumbers = {0,460,10,10};
	SDL_Rect desnumbers = {18,178,10,10};
	SDL_Rect srctext = {0,0,140,20};
	SDL_Rect destext = {115,176,140,20};
	int i = 0;

	/* Show heart and crosses sprites */
	if (changetiles == 1)
		srcbar.y = 224;
	blit(renderer,tiles,&srcbar,&desbar);
	srcbar.x = 461;
	srcbar.w = 12;
	desbar.x = 32;
	blit(renderer,tiles,&srcbar,&desbar);

	for (i=0; i<=2; i++) {
		switch (i) {
			case 0: srcnumbers.x = lifes * 10;
			        blit(renderer,fonts,&srcnumbers,&desnumbers);
							break;
			case 1: if (crosses < 10) {
								desnumbers.x = 50;
								srcnumbers.x = crosses * 10;
								blit(renderer,fonts,&srcnumbers,&desnumbers);
							}
							else {
								desnumbers.x = 50;
								srcnumbers.x = 10;
								blit(renderer,fonts,&srcnumbers,&desnumbers);
								desnumbers.x = 55;
								srcnumbers.x = (crosses - 10) * 10;
								blit(renderer,fonts,&srcnumbers,&desnumbers);
							}
							break;
			case 2: if ((room[0] > 0) && (room[0] < 4)) {
								srctext.y = (room[0] - 1) * 20;
								blit(renderer,fonts,&srctext,&destext);
							}
							if (room[0] > 4) {
								srctext.y = (room[0] - 2) * 20;
								blit(renderer,fonts,&srctext,&destext);
							}
							break;
		}

	}

}

void drawrope (struct enem enemies,SDL_Surface *renderer,SDL_Surface *tiles,uint changetiles) {

	int i = 0;
	int blocks = 0;
	int j = 0;
	SDL_Rect srctile = {424,8,16,8};
	SDL_Rect destile = {0,0,16,8};

	for (i=2; i<6; i++) {
		blocks = (enemies.y[i] - (enemies.limleft[i] - 8)) / 8;
		for (j=0; j<=blocks; j++) {
			srctile.y = 8 + (changetiles * 120);
	  	destile.x = enemies.x[i];
	  	destile.y = (enemies.limleft[i] - 8) + (8 * j);
	  	blit_colorkey(renderer,tiles,&srctile,&destile);
		}
	}

}

void drawshoots (float proyec[],SDL_Surface *tiles,SDL_Surface *renderer,struct enem *enemies,uint changetiles) {
/* Shoots from skeletons & gargoyles */

	SDL_Rect srctile = {656,24,16,8};
	SDL_Rect destile = {0,0,0,0};
	int i = 0;
	int n = 0;

	srctile.y = 24 + (changetiles * 120);

  for (n=0; n<=4; n+=2) {
		if (proyec[n] > 0) {
	  	i = proyec[n+1];
	  	if (enemies->type[i] == 15) {
				srctile.h = 16;
				srctile.x = 640 - (16 * enemies->direction[i]);
	  	}

	  	/* Move shoot */
	  	if (enemies->direction[i] == 1) {
				if (proyec[n] > enemies->limleft[i])
				  proyec[n] -= 2.5;
				else {
				  enemies->fire[i] = 0;
				  enemies->speed[i] = 0;
				  proyec[n] = 0;
				}
	  	}
	  	else {
				if (proyec[n] < enemies->limright[i])
		  		proyec[n] += 2.5;
				else {
		  		enemies->fire[i] = 0;
				  enemies->speed[i] = 0;
				  proyec[n] = 0;
				}
	  	}
	  	destile.w = srctile.w;
			destile.h = srctile.h;

	  	/* Draw shoot */
	  	switch (enemies->direction[i]) {
				case 0: if ((proyec[n] < (enemies->limright[i] - 8)) && (proyec[n] != 0)) {
								  destile.x = proyec[n];
								  destile.y = enemies->y[i] + 8;
								  blit_colorkey(renderer,tiles,&srctile,&destile);
								}
								break;
				case 1: if (proyec[n] > (enemies->limleft[i] + 8)) {
								  destile.x = proyec[n];
								  destile.y = enemies->y[i] + 8;
								  blit_colorkey(renderer,tiles,&srctile,&destile);
								}
								break;
	  	}
		}

	}

}

extern unsigned int _binary_graphics_parchment1_png_start;
extern unsigned int _binary_graphics_parchment1_png_end;
extern unsigned int _binary_graphics_parchment2_png_start;
extern unsigned int _binary_graphics_parchment2_png_end;
extern unsigned int _binary_graphics_parchment3_png_start;
extern unsigned int _binary_graphics_parchment3_png_end;
extern unsigned int _binary_graphics_parchment4_png_start;
extern unsigned int _binary_graphics_parchment4_png_end;
extern unsigned int _binary_graphics_parchment5_png_start;
extern unsigned int _binary_graphics_parchment5_png_end;
extern unsigned int _binary_graphics_parchment6_png_start;
extern unsigned int _binary_graphics_parchment6_png_end;

void showparchment (SDL_Renderer *renderer,uint *parchment) {

    SDL_RWops *rw = NULL;

	switch (*parchment) {
		case 3:
		    rw = SDL_RWFromMem(&_binary_graphics_parchment1_png_start,
		        (unsigned int)&_binary_graphics_parchment1_png_end - (unsigned int)&_binary_graphics_parchment1_png_start);
			break;
		case 8:
            rw = SDL_RWFromMem(&_binary_graphics_parchment2_png_start,
                (unsigned int)&_binary_graphics_parchment2_png_end - (unsigned int)&_binary_graphics_parchment2_png_start);
			break;
		case 12:
            rw = SDL_RWFromMem(&_binary_graphics_parchment3_png_start,
                (unsigned int)&_binary_graphics_parchment3_png_end - (unsigned int)&_binary_graphics_parchment3_png_start);
			break;
		case 14:
            rw = SDL_RWFromMem(&_binary_graphics_parchment4_png_start,
                (unsigned int)&_binary_graphics_parchment4_png_end - (unsigned int)&_binary_graphics_parchment4_png_start);
			break;
		case 16:
            rw = SDL_RWFromMem(&_binary_graphics_parchment5_png_start,
                (unsigned int)&_binary_graphics_parchment5_png_end - (unsigned int)&_binary_graphics_parchment5_png_start);
			break;
		case 21:
            rw = SDL_RWFromMem(&_binary_graphics_parchment6_png_start,
                (unsigned int)&_binary_graphics_parchment6_png_end - (unsigned int)&_binary_graphics_parchment6_png_start);
			break;

	}

	if (rw != NULL) {
	    SDL_Texture *yparchment = IMG_LoadTexture_RW(renderer,rw,1);
	    SDL_RenderCopy(renderer,yparchment,NULL,NULL);
	    SDL_DestroyTexture(yparchment);
	}

}

extern unsigned int _binary_graphics_redparch_png_start;
extern unsigned int _binary_graphics_redparch_png_end;

void redparchment (SDL_Renderer *renderer,struct hero *jean) {

    SDL_RWops *rw = SDL_RWFromMem(&_binary_graphics_redparch_png_start,
        (unsigned int)&_binary_graphics_redparch_png_end - (unsigned int)&_binary_graphics_redparch_png_start);
	SDL_Texture *rparchment = IMG_LoadTexture_RW(renderer,rw,1);
	SDL_RenderCopy(renderer,rparchment,NULL,NULL);
	SDL_DestroyTexture(rparchment);

	jean->flags[6] = 4;

}

extern unsigned int _binary_graphics_blueparch_png_start;
extern unsigned int _binary_graphics_blueparch_png_end;

void blueparchment (SDL_Renderer *renderer,struct hero *jean) {

    SDL_RWops *rw = SDL_RWFromMem(&_binary_graphics_blueparch_png_start,
        (unsigned int)&_binary_graphics_blueparch_png_end - (unsigned int)&_binary_graphics_blueparch_png_start);
	SDL_Texture *bparchment = IMG_LoadTexture_RW(renderer,rw,1);
	SDL_RenderCopy(renderer,bparchment,NULL,NULL);
	SDL_DestroyTexture(bparchment);

}
