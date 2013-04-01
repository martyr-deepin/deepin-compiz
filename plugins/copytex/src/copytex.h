/*
 * Compiz copy to texture plugin
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@compiz-fusion.org
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
 */

/*
 * This plugin uses the "copy to texture" taken from Luminocity
 * http://live.gnome.org/Luminocity
 */

#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include <boost/shared_ptr.hpp>

#include <X11/extensions/XShm.h>

#include <sys/shm.h>
#include <sys/ipc.h>

extern const int MAX_SUB_TEX;
extern const unsigned int SHM_SIZE;

class CopyTexture;

class CopyPixmap {
    public:
	typedef std::vector <CopyTexture *> Textures;
	typedef boost::shared_ptr <CopyPixmap> Ptr;

	static CopyPixmap::Ptr
	create (Pixmap pixmap,
		int    width,
		int    height,
		int    depth);

	~CopyPixmap ();

	static GLTexture::List bindPixmapToTexture (Pixmap                       pixmap,
						    int                          width,
						    int                          height,
						    int                          depth,
						    compiz::opengl::PixmapSource source);

    public:
	Textures textures;

	Pixmap pixmap;
	Damage damage;
	int    depth;

    private:

	CopyPixmap (Pixmap pixmap,
		    int width,
		    int height,
		    int depth);
};

class CopyTexture : public GLTexture {
    public:
	CopyTexture (boost::shared_ptr <CopyPixmap> cp, CompRect dim);
	~CopyTexture ();

	void enable (Filter filter);
	void disable ();
	void update ();

	const CompRect & size () const
	{
	    return dim;
	}

    public:
	CopyPixmap::Ptr cp;
	CompRect        dim;
	CompRect        damage;
};

class CopytexScreen :
    public ScreenInterface,
    public PluginClassHandler<CopytexScreen,CompScreen>
{
    public:
	CopytexScreen (CompScreen *screen);
	~CopytexScreen ();

	void handleEvent (XEvent *);

	bool            useShm;
	XShmSegmentInfo shmInfo;

	int damageNotify;

	std::map <Damage, CopyPixmap::Ptr> pixmaps;

	GLTexture::BindPixmapHandle hnd;
};

#define COPY_SCREEN(s) \
    CopytexScreen *cs = CopytexScreen::get (s)

class CopytexPluginVTable :
    public CompPlugin::VTableForScreen<CopytexScreen>
{
    public:
	bool init ();

};
