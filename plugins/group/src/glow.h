/**
 *
 * Compiz group plugin
 *
 * glow.h
 *
 * Copyright : (C) 2006-2010 by Patrick Niklaus, Roi Cohen,
 * 				Danny Baumann, Sam Spilsbury
 * Authors: Patrick Niklaus <patrick.niklaus@googlemail.com>
 *          Roi Cohen       <roico.beryl@gmail.com>
 *          Danny Baumann   <maniac@opencompositing.org>
 * 	    Sam Spilsbury   <smspillaz@gmail.com>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 **/
 
#ifndef _GROUP_GLOW_H
#define _GROUP_GLOW_H

#include "group.h"
 
extern const unsigned short GLOWQUAD_TOPLEFT;
extern const unsigned short GLOWQUAD_TOPRIGHT;
extern const unsigned short GLOWQUAD_BOTTOMLEFT;
extern const unsigned short GLOWQUAD_BOTTOMRIGHT;
extern const unsigned short GLOWQUAD_TOP;
extern const unsigned short GLOWQUAD_BOTTOM;
extern const unsigned short GLOWQUAD_LEFT;
extern const unsigned short GLOWQUAD_RIGHT;
extern const unsigned short NUM_GLOWQUADS;

/* Represents a particular glow texture, so here
 * we have hardcoded in the texture data, the offset
 * and the size of the texture
 */

typedef struct _GlowTextureProperties {
    char *textureData;
    int  textureSize;
    int  glowOffset;
} GlowTextureProperties;

/* Each glow quad contains a 2x2 scale + positional matrix
 * (the 3rd column is not used since that is for matrix skew
 *  operations which we do not care about)
 * and also a CompRect which describes the size and position of
 * the quad on the glow
 */

class GlowQuad {
    public:
	CompRect	  mBox;
	GLTexture::Matrix mMatrix;
};

extern const GlowTextureProperties glowTextureProperties[2];

#endif
