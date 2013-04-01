/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 *          David Reveman <davidr@novell.com>
 */

#ifndef _COMPSCREEN_H
#define _COMPSCREEN_H

#include <core/window.h>
#include <core/output.h>
#include <core/session.h>
#include <core/plugin.h>
#include <core/match.h>
#include <core/pluginclasses.h>
#include <core/region.h>
#include <core/modifierhandler.h>
#include <core/valueholder.h>

#include <boost/scoped_ptr.hpp>

class CompScreenImpl;
class PrivateScreen;
class CompManager;
class CoreWindow;
class CoreOptions;
class ServerGrabInterface;

typedef std::list<CompWindow *> CompWindowList;
typedef std::vector<CompWindow *> CompWindowVector;

extern bool       replaceCurrentWm;
extern bool       debugOutput;

extern CompScreen   *screen;

extern ModifierHandler *modHandler;

extern int lastPointerX;
extern int lastPointerY;
extern unsigned int lastPointerMods;
extern int pointerX;
extern int pointerY;
extern unsigned int pointerMods;

#define NOTIFY_CREATE_MASK (1 << 0)
#define NOTIFY_DELETE_MASK (1 << 1)
#define NOTIFY_MOVE_MASK   (1 << 2)
#define NOTIFY_MODIFY_MASK (1 << 3)

#define SCREEN_EDGE_LEFT	    0
#define SCREEN_EDGE_RIGHT	    1
#define SCREEN_EDGE_TOP		    2
#define SCREEN_EDGE_BOTTOM	    3
#define SCREEN_EDGE_TOPLEFT	    4
#define SCREEN_EDGE_TOPRIGHT	    5
#define SCREEN_EDGE_BOTTOMLEFT	    6
#define SCREEN_EDGE_BOTTOMRIGHT	    7
#define SCREEN_EDGE_NUM		    8

typedef boost::function<void (short int)> FdWatchCallBack;
typedef boost::function<void (const char *)> FileWatchCallBack;

typedef int CompFileWatchHandle;
typedef int CompWatchFdHandle;

/**
 * Information needed to invoke a CallBack when a file changes.
 */
struct CompFileWatch {
    CompString		path;
    int			mask;
    FileWatchCallBack   callBack;
    CompFileWatchHandle handle;
};
typedef std::list<CompFileWatch *> CompFileWatchList;

#define ACTIVE_WINDOW_HISTORY_SIZE 64
#define ACTIVE_WINDOW_HISTORY_NUM  32

/**
 * Information about the last activity with a window.
 */
struct CompActiveWindowHistory {
    Window id[ACTIVE_WINDOW_HISTORY_SIZE];
    int    x;
    int    y;
    int    activeNum;
};

/**
 * Interface to an abstract screen.
 */
class ScreenInterface : public WrapableInterface<CompScreen, ScreenInterface> {
    public:
	virtual void fileWatchAdded (CompFileWatch *fw);
	virtual void fileWatchRemoved (CompFileWatch *fw);

	virtual bool initPluginForScreen (CompPlugin *p);
	virtual void finiPluginForScreen (CompPlugin *p);

	virtual bool setOptionForPlugin (const char *plugin,
					 const char *name,
					 CompOption::Value &v);

	virtual void sessionEvent (CompSession::Event event,
				   CompOption::Vector &options);

	virtual void handleEvent (XEvent *event);
        virtual void handleCompizEvent (const char * plugin, const char *event,
					CompOption::Vector &options);

        virtual bool fileToImage (CompString &path, CompSize &size,
				  int &stride, void *&data);
	virtual bool imageToFile (CompString &path, CompString &format,
				  CompSize &size, int stride, void *data);

	virtual CompMatch::Expression *matchInitExp (const CompString& value);

	virtual void matchExpHandlerChanged ();
	virtual void matchPropertyChanged (CompWindow *window);

	virtual void logMessage (const char   *componentName,
				 CompLogLevel level,
				 const char   *message);

	virtual void enterShowDesktopMode ();
	virtual void leaveShowDesktopMode (CompWindow *window);

	virtual void outputChangeNotify ();
	virtual void addSupportedAtoms (std::vector<Atom>& atoms);

};

namespace compiz { namespace private_screen {
    struct Grab;
    class EventManager;
    class Ping;
}}

namespace compiz {
class DesktopWindowCount
{
public:
    virtual void incrementDesktopWindowCount() = 0;
    virtual void decrementDesktopWindowCount() = 0;
    virtual int desktopWindowCount() = 0;
protected:
    ~DesktopWindowCount() {}
};

class MapNum
{
public:
    virtual unsigned int nextMapNum() = 0;
protected:
    ~MapNum() {}
};

class Ping
{
public:
    virtual unsigned int lastPing () const = 0;
protected:
    ~Ping() {}
};

class XWindowInfo
{
public:
    virtual int getWmState (Window id) = 0;
    virtual void setWmState (int state, Window id) const = 0;
    virtual void getMwmHints (Window id,
			  unsigned int *func,
			  unsigned int *decor) const = 0;
    virtual unsigned int getProtocols (Window id) = 0;
    virtual unsigned int getWindowType (Window id) = 0;
    virtual unsigned int getWindowState (Window id) = 0;
protected:
    ~XWindowInfo() {}
};

class History
{
public:
    virtual unsigned int activeNum () const = 0;
    virtual CompActiveWindowHistory *currentHistory () = 0;
protected:
    ~History() {}
};

}

class CompScreen :
    public WrapableHandler<ScreenInterface, 18>,
    public PluginClassStorage, // TODO should be an interface here
    public CompSize,
    public virtual ::compiz::DesktopWindowCount,
    public virtual ::compiz::MapNum,
    public virtual ::compiz::Ping,
    public virtual ::compiz::XWindowInfo,
    public virtual ::compiz::History,
    public CompOption::Class   // TODO should be an interface here
{
public:
    typedef compiz::private_screen::Grab* GrabHandle;

    WRAPABLE_HND (0, ScreenInterface, void, fileWatchAdded, CompFileWatch *)
    WRAPABLE_HND (1, ScreenInterface, void, fileWatchRemoved, CompFileWatch *)

    WRAPABLE_HND (2, ScreenInterface, bool, initPluginForScreen,
		  CompPlugin *)
    WRAPABLE_HND (3, ScreenInterface, void, finiPluginForScreen,
		  CompPlugin *)

    WRAPABLE_HND (4, ScreenInterface, bool, setOptionForPlugin,
		  const char *, const char *, CompOption::Value &)

    WRAPABLE_HND (5, ScreenInterface, void, sessionEvent, CompSession::Event,
		  CompOption::Vector &)
    WRAPABLE_HND (6, ScreenInterface, void, handleEvent, XEvent *event)
    WRAPABLE_HND (7, ScreenInterface, void, handleCompizEvent,
		  const char *, const char *, CompOption::Vector &)

    WRAPABLE_HND (8, ScreenInterface, bool, fileToImage, CompString &,
		  CompSize &, int &, void *&);
    WRAPABLE_HND (9, ScreenInterface, bool, imageToFile, CompString &,
		  CompString &, CompSize &, int, void *);

    WRAPABLE_HND (10, ScreenInterface, CompMatch::Expression *,
		  matchInitExp, const CompString&);
    WRAPABLE_HND (11, ScreenInterface, void, matchExpHandlerChanged)
    WRAPABLE_HND (12, ScreenInterface, void, matchPropertyChanged,
		  CompWindow *)

    WRAPABLE_HND (13, ScreenInterface, void, logMessage, const char *,
		  CompLogLevel, const char*)
    WRAPABLE_HND (14, ScreenInterface, void, enterShowDesktopMode);
    WRAPABLE_HND (15, ScreenInterface, void, leaveShowDesktopMode,
		  CompWindow *);

    WRAPABLE_HND (16, ScreenInterface, void, outputChangeNotify);
    WRAPABLE_HND (17, ScreenInterface, void, addSupportedAtoms,
		  std::vector<Atom>& atoms);

    unsigned int allocPluginClassIndex ();
    void freePluginClassIndex (unsigned int index);
    static int checkForError (Display *dpy);


    // Interface hoisted from CompScreen
    virtual bool updateDefaultIcon () = 0;
    virtual Display * dpy () = 0;
    virtual Window root () = 0;
    virtual const CompSize  & vpSize () const = 0;
    virtual void forEachWindow (CompWindow::ForEach) =0;
    virtual CompWindowList & windows () = 0;
    virtual void moveViewport (int tx, int ty, bool sync) = 0;
    virtual const CompPoint & vp () const = 0;
    virtual void updateWorkarea () = 0;
    virtual bool addAction (CompAction *action) = 0;
    virtual CompWindow * findWindow (Window id) = 0;

    virtual CompWindow * findTopLevelWindow (
	    Window id, bool   override_redirect = false) = 0;
    virtual void toolkitAction (
	    Atom   toolkitAction,
	    Time   eventTime,
	    Window window,
	    long   data0,
	    long   data1,
	    long   data2) = 0;
    virtual unsigned int showingDesktopMask() const = 0;

    virtual bool grabsEmpty() const = 0;
    virtual void sizePluginClasses(unsigned int size) = 0;
    virtual CompOutput::vector & outputDevs () = 0;
    virtual void setWindowState (unsigned int state, Window id) = 0;
    virtual bool XShape () = 0;
    virtual std::vector<XineramaScreenInfo> & screenInfo () = 0;
    virtual CompWindowList & serverWindows () = 0;
    virtual void setWindowProp (Window       id,
			    Atom         property,
			    unsigned int value) = 0;
    virtual Window activeWindow () = 0;
    virtual unsigned int currentDesktop () = 0;
    virtual void focusDefaultWindow () = 0;
    virtual Time getCurrentTime () = 0;
    virtual unsigned int getWindowProp (Window       id,
				    Atom         property,
				    unsigned int defaultValue) = 0;
    virtual void insertServerWindow (CompWindow *w, Window aboveId) = 0;
    virtual void insertWindow (CompWindow *w, Window aboveId) = 0;
    virtual unsigned int nDesktop () = 0;
    virtual int outputDeviceForGeometry (const CompWindow::Geometry& gm) = 0;
    virtual int screenNum () = 0;
    virtual void unhookServerWindow (CompWindow *w) = 0;
    virtual void unhookWindow (CompWindow *w) = 0;
    virtual void viewportForGeometry (const CompWindow::Geometry &gm,
				  CompPoint                   &viewport) = 0;

    virtual void addToDestroyedWindows(CompWindow * cw) = 0;
    virtual const CompRect & workArea () const = 0;
    virtual void removeAction (CompAction *action) = 0;
    virtual CompOption::Vector & getOptions () = 0;
    virtual bool setOption (const CompString &name, CompOption::Value &value) = 0;
    virtual void storeValue (CompString key, CompPrivate value) = 0;
    virtual bool hasValue (CompString key) = 0;
    virtual CompPrivate getValue (CompString key) = 0;
    virtual void eraseValue (CompString key) = 0;
    virtual CompWatchFdHandle addWatchFd (int             fd,
				      short int       events,
				      FdWatchCallBack callBack) = 0;
    virtual void removeWatchFd (CompWatchFdHandle handle) = 0;
    virtual void eventLoop () = 0;

    virtual CompFileWatchHandle addFileWatch (const char        *path,
					  int               mask,
					  FileWatchCallBack callBack) = 0;
    virtual void removeFileWatch (CompFileWatchHandle handle) = 0;
    virtual const CompFileWatchList& getFileWatches () const = 0;
    virtual void updateSupportedWmHints () = 0;

    virtual CompWindowList & destroyedWindows () = 0;
    virtual const CompRegion & region () const = 0;
    virtual bool hasOverlappingOutputs () = 0;
    virtual CompOutput & fullscreenOutput () = 0;
    virtual void setWindowProp32 (Window         id,
			      Atom           property,
			      unsigned short value) = 0;
    virtual unsigned short getWindowProp32 (Window         id,
					Atom           property,
					unsigned short defaultValue) = 0;
    virtual bool readImageFromFile (CompString &name,
				CompString &pname,
				CompSize   &size,
				void       *&data) = 0;
    virtual XWindowAttributes attrib () = 0;
    virtual CompIcon *defaultIcon () const = 0;
    virtual bool otherGrabExist (const char *, ...) = 0;
    virtual GrabHandle pushGrab (Cursor cursor, const char *name) = 0;
    virtual void removeGrab (GrabHandle handle, CompPoint *restorePointer) = 0;
    virtual bool writeImageToFile (CompString &path,
			       const char *format,
			       CompSize   &size,
			       void       *data) = 0;
    virtual void runCommand (CompString command) = 0;
    virtual bool shouldSerializePlugins () = 0;
    virtual const CompRect & getWorkareaForOutput (unsigned int outputNum) const = 0;
    virtual CompOutput & currentOutputDev () const = 0;
    virtual bool grabExist (const char *) = 0;
    virtual Cursor invisibleCursor () = 0;
    virtual void sendWindowActivationRequest (Window id) = 0;
    virtual const CompWindowVector & clientList (bool stackingOrder = true) = 0;
    virtual int outputDeviceForPoint (const CompPoint &point) = 0;
    virtual int outputDeviceForPoint (int x, int y) = 0;
    virtual int xkbEvent () = 0;
    virtual void warpPointer (int dx, int dy) = 0;
    virtual void updateGrab (GrabHandle handle, Cursor cursor) = 0;
    virtual int shapeEvent () = 0;

    virtual int syncEvent () = 0;
    virtual Window autoRaiseWindow () = 0;

    virtual const char * displayString () = 0;
    virtual CompRect getCurrentOutputExtents () = 0;
    virtual Cursor normalCursor () = 0;
    virtual bool grabbed () = 0;
    virtual SnDisplay * snDisplay () = 0;

    virtual void processEvents () = 0;
    virtual void alwaysHandleEvent (XEvent *event) = 0;

    virtual ServerGrabInterface * serverGrabInterface () = 0;

    // Replacements for friends accessing priv. They are declared virtual to
    // ensure the ABI is stable if/when they are moved to CompScreenImpl.
    // They are only intended for use within compiz-core
    virtual bool displayInitialised() const = 0;
    virtual void updatePassiveKeyGrabs () const = 0;
    virtual void applyStartupProperties (CompWindow *window) = 0;
    virtual void updateClientList() = 0;
    virtual CompWindow * getTopWindow() const = 0;
    virtual CompWindow * getTopServerWindow() const = 0;
    virtual CoreOptions& getCoreOptions() = 0;
    virtual Colormap colormap() const = 0;
    virtual void setCurrentDesktop (unsigned int desktop) = 0;
    virtual Window activeWindow() const = 0;
    virtual void updatePassiveButtonGrabs(Window serverFrame) = 0;
    virtual bool grabWindowIsNot(Window w) const = 0;
    virtual void incrementPendingDestroys() = 0;
    virtual void setNextActiveWindow(Window id) = 0;
    virtual Window getNextActiveWindow() const = 0;
    virtual CompWindow * focusTopMostWindow () = 0;
    // End of "internal use only" functions

protected:
	CompScreen();

private:
    // The "wrapable" functions delegate to these (for mocking)
    virtual bool _initPluginForScreen(CompPlugin *) = 0;
    virtual void _finiPluginForScreen(CompPlugin *) = 0;
    virtual bool _setOptionForPlugin(const char *, const char *, CompOption::Value &) = 0;
    virtual void _handleEvent(XEvent *event) = 0;
    virtual void _logMessage(const char *, CompLogLevel, const char*) = 0;
    virtual void _enterShowDesktopMode() = 0;
    virtual void _leaveShowDesktopMode(CompWindow *) = 0;
    virtual void _addSupportedAtoms(std::vector<Atom>& atoms) = 0;

    virtual void _fileWatchAdded(CompFileWatch *) = 0;
    virtual void _fileWatchRemoved(CompFileWatch *) = 0;
    virtual void _sessionEvent(CompSession::Event, CompOption::Vector &) = 0;
    virtual void _handleCompizEvent(const char *, const char *, CompOption::Vector &) = 0;
    virtual bool _fileToImage(CompString &, CompSize &, int &, void *&) = 0;
    virtual bool _imageToFile(CompString &, CompString &, CompSize &, int, void *) = 0;
    virtual CompMatch::Expression * _matchInitExp(const CompString&) = 0;
    virtual void _matchExpHandlerChanged() = 0;
    virtual void _matchPropertyChanged(CompWindow *) = 0;
    virtual void _outputChangeNotify() = 0;
};

#endif
