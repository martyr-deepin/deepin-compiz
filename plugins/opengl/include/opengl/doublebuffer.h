#ifndef _COMPIZ_OPENGL_BUFFERBLIT_H
#define _COMPIZ_OPENGL_BUFFERBLIT_H

#include <core/region.h>

namespace compiz
{
namespace opengl
{

class DoubleBuffer
{
    public:
	DoubleBuffer ();
	virtual ~DoubleBuffer ();

	virtual void swap () const = 0;
	virtual bool blitAvailable () const = 0;
	virtual void blit (const CompRegion &region) const = 0;
	virtual bool fallbackBlitAvailable () const = 0;
	virtual void fallbackBlit (const CompRegion &region) const = 0;
	virtual void copyFrontToBack () const = 0;

	typedef enum
	{
	    VSYNC,
	    HAVE_PERSISTENT_BACK_BUFFER,
	    NEED_PERSISTENT_BACK_BUFFER,
	    _NSETTINGS
	} Setting;

	void set (Setting name, bool value);
	void render (const CompRegion &region, bool fullscreen);

    protected:
	bool setting[_NSETTINGS];
};

}
}
#endif
