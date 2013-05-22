#ifndef _COMPIZ_OPENGL_BUFFERBLIT_H
#define _COMPIZ_OPENGL_BUFFERBLIT_H

#include <core/region.h>
#include <boost/function.hpp>

namespace compiz
{
namespace opengl
{

namespace impl
{
typedef boost::function <int (int, int, unsigned int *)> GLXWaitVideoSyncSGIFunc;
typedef boost::function <void (int)> GLXSwapIntervalEXTFunc;
}

class DoubleBuffer
{
    public:
	DoubleBuffer (const impl::GLXSwapIntervalEXTFunc  &swapIntervalFunc,
		      const impl::GLXWaitVideoSyncSGIFunc &waitVideoSyncFunc);
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

	typedef enum _RedrawType
	{
	    Swap,
	    Blit
	} FrontbufferRedrawType;

	typedef enum _SyncType
	{
	    NoSync = 0,
	    Async = 1,
	    Blocking = 2
	} SyncType;

	typedef enum _FrameThrottleState
	{
	    ExternalFrameThrottlingRequired,
	    FrameThrottledInternally
	} FrameThrottleState;

	void set (Setting name, bool value);
	void render (const CompRegion &region, bool fullscreen);
	void vsync (FrontbufferRedrawType redrawType);

	bool hardwareVSyncFunctional ();

    protected:
	bool setting[_NSETTINGS];

    private:

	virtual bool enableAsyncVideoSync (FrontbufferRedrawType, FrameThrottleState &);
	virtual void disableAsyncVideoSync ();
	virtual bool enableBlockingVideoSync (FrontbufferRedrawType, FrameThrottleState &);
	virtual void disableBlockingVideoSync ();

	SyncType		      syncType;

	FrameThrottleState            bufferFrameThrottleState;
	unsigned int                  blockingVSyncUnthrottledFrames;

	impl::GLXSwapIntervalEXTFunc  swapIntervalFunc;
	impl::GLXWaitVideoSyncSGIFunc waitVideoSyncFunc;
	unsigned int                  lastVSyncCounter;
};

}
}
#endif
