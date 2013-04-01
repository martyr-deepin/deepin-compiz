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

#ifndef _PRIVATESCREEN_H
#define _PRIVATESCREEN_H

#include <core/screen.h>
#include <core/size.h>
#include <core/point.h>
#include <core/timer.h>
#include <core/plugin.h>
#include <core/servergrab.h>
#include <time.h>
#include <boost/shared_ptr.hpp>

#include <glibmm/main.h>

#include "privatetimeoutsource.h"
#include "privateiosource.h"
#include "privateeventsource.h"
#include "privatesignalsource.h"

#include "core_options.h"

#include <set>

namespace compiz { namespace private_screen
{

class OutputDevices
{
public:
    OutputDevices();

    void setCurrentOutput(unsigned int outputNum);

    CompOutput& getCurrentOutputDev() { return outputDevs[currentOutputDev]; }

    bool hasOverlappingOutputs() const { return overlappingOutputs; }

    void computeWorkAreas(CompRect& workArea, bool& workAreaChanged,
	    CompRegion& allWorkArea, const CompWindowList& windows);

    const CompOutput& getOutputDev(unsigned int outputNum) const
    { return outputDevs[outputNum]; }

    // TODO breaks encapsulation horribly ought to be const at least
    // Even better, use begin() and end() return const_iterators
    // BUT this is exported directly through API - which makes changing
    // it a PITA.
    CompOutput::vector& getOutputDevs() { return outputDevs; }

    int outputDeviceForGeometry(const CompWindow::Geometry& gm, int strategy,
	    CompScreen* screen) const;
    void updateOutputDevices(CoreOptions& coreOptions, CompScreen* screen);

private:
    void setGeometryOnDevice(unsigned int nOutput, int x, int y,
	    const int width, const int height);
    void adoptDevices(unsigned int nOutput);

    static CompRect computeWorkareaForBox(const CompRect& box,
	    const CompWindowList& windows);

    CompOutput::vector outputDevs;
    bool               overlappingOutputs;
    int	           currentOutputDev;
};

}} //::compiz::private_screen

CompPlugin::VTable * getCoreVTable ();

class CoreWindow;

extern bool shutDown;
extern bool restartSignal;

extern bool	  useDesktopHints;

extern std::list <CompString> initialPlugins;


typedef struct _CompDelayedEdgeSettings
{
    CompAction::CallBack initiate;
    CompAction::CallBack terminate;

    unsigned int edge;
    unsigned int state;

    CompOption::Vector options;
} CompDelayedEdgeSettings;


struct CompScreenEdge {
    Window	 id;
    unsigned int count;
};

struct CompGroup {
    unsigned int      refCnt;
    Window	      id;
};

struct CompStartupSequence {
    SnStartupSequence		*sequence;
    unsigned int		viewportX;
    unsigned int		viewportY;
};

namespace compiz
{
namespace core
{
namespace screen
{
    inline int wraparound_mod (int a, int b)
    {
	if (a < 0)
	    return (b - ((-a - 1) % (b))) - 1;
	else
	    return a % b;
    };
}
}
}

namespace compiz
{
namespace private_screen
{
class History;

class WindowManager : boost::noncopyable
{
    public:

	typedef CompWindow::ForEach Functor;

	WindowManager();

	CompGroup * addGroup (Window id);
	void removeGroup (CompGroup *group);
	CompGroup * findGroup (Window id);

	void eraseWindowFromMap (Window id);
	void removeDestroyed ();

	void updateClientList (PrivateScreen& ps);

	void addToDestroyedWindows(CompWindow * cw)
	    { destroyedWindows.push_back (cw); }

	void incrementPendingDestroys() { pendingDestroys++; }
	const CompWindowVector& getClientList () const
	    { return clientList; }
	const CompWindowVector& getClientListStacking () const
	    { return clientListStacking; }

	CompWindow * findWindow (Window id) const;
	CompWindow * getTopWindow() const;
	CompWindow * getTopServerWindow() const;


	void removeFromFindWindowCache(CompWindow* w)
	{
	    if (w == lastFoundWindow)
		lastFoundWindow = 0;
	}

	void addWindowToMap(CompWindow* w)
	{
	    if (w->id () != 1)
		windowsMap[w->id ()] = w;
	}

	void validateServerWindows();

	void invalidateServerWindows();

	void insertWindow (CompWindow* w, Window aboveId);
	void unhookWindow (CompWindow *w);
	CompWindowList& getWindows()	{ return windows; }

	CompWindowList& getDestroyedWindows()	{ return destroyedWindows; }

	void insertServerWindow(CompWindow* w, Window aboveId);
	void unhookServerWindow(CompWindow *w);
	CompWindowList& getServerWindows()	{ return serverWindows; }

	typedef CompWindowList::const_iterator iterator;
	typedef CompWindowList::const_reverse_iterator reverse_iterator;

	iterator begin() const { return windows.begin(); }
	iterator end() const { return windows.end(); }
	reverse_iterator rbegin() const { return windows.rbegin(); }
	reverse_iterator rend() const { return windows.rend(); }

	void clearFullscreenHints() const;
	void showOrHideForDesktop(unsigned int desktop) const;
	void setWindowActiveness(::compiz::private_screen::History& history) const;
	void setNumberOfDesktops (unsigned int nDesktop) const;
	void updateWindowSizes() const;

	void forEachWindow(Functor const& f) const
	{
	    std::for_each(windows.begin(), windows.end(), f);
	}

    private:
	CompWindowList windows;
	CompWindowList serverWindows;
	CompWindowList destroyedWindows;
	bool           stackIsFresh;

	CompWindow::Map windowsMap;
	std::list<CompGroup *> groups;

	CompWindowVector clientList;            /* clients in mapping order */
	CompWindowVector clientListStacking;    /* clients in stacking order */

	std::vector<Window> clientIdList;        /* client ids in mapping order */
	std::vector<Window> clientIdListStacking;/* client ids in stacking order */

	unsigned int pendingDestroys;

	mutable CompWindow* lastFoundWindow;
};

unsigned int windowStateFromString (const char *str);

class PluginManager
{
    public:
	PluginManager();

	void updatePlugins (CompScreen* screen, CompOption::Value::Vector const& extraPluginsRequested);

	void setPlugins(CompOption::Value::Vector const& vList)
	{
	    plugin.set (CompOption::TypeString, vList);
	}

	bool isDirtyPluginList () const { return dirtyPluginList; }
	void setDirtyPluginList () { dirtyPluginList = true; }

	CompOption::Value::Vector mergedPluginList(CompOption::Value::Vector const& extraPluginsRequested) const;

    private:
	CompOption::Value plugin;
	bool	          dirtyPluginList;
	typedef std::set<CompString> CompStringSet;
	CompStringSet blacklist;
};

class GrabList
{
    // TODO: std::list<Grab *> is almost certainly the wrong data
    // structure. Probably better as std::vector<Grab> - many fewer
    // memory allocations and releases.
    typedef std::list<Grab *> GrabPtrList;

public:
    typedef GrabPtrList::iterator GrabIterator;

    bool grabsEmpty() const { return grabs.empty(); }
    void grabsPush(Grab* grab) { grabs.push_back (grab); }
    GrabIterator grabsBegin() { return grabs.begin(); }
    GrabIterator grabsEnd() { return grabs.end(); }
    void grabsRemove(Grab* grab);
    bool grabExist (const char *grab);
    Grab* grabsBack() { return grabs.back (); }

private:
    GrabPtrList grabs;
};

class EventManager :
    public GrabList
{
    public:
	EventManager ();
	~EventManager ();

	void init ();

	void handleSignal (int signum);
	bool triggerPress   (CompAction         *action,
			     CompAction::State   state,
			     CompOption::Vector &arguments);
	bool triggerRelease (CompAction         *action,
	                     CompAction::State   state,
	                     CompOption::Vector &arguments);

	void startEventLoop(Display* dpy);
	void quit() { mainloop->quit(); }

	CompWatchFdHandle addWatchFd (
	    int             fd,
	    short int       events,
	    FdWatchCallBack callBack);

	void removeWatchFd (CompWatchFdHandle handle);

	CompFileWatch* addFileWatch (
	    const char        *path,
	    int               mask,
	    FileWatchCallBack callBack);

	CompFileWatch* removeFileWatch (CompFileWatchHandle handle);

	const CompFileWatchList& getFileWatches () const;

	void grabNotified() { grabbed = true; }
	void ungrabNotified() { grabbed = false; }
	bool isGrabbed() const { return grabbed; }

	void setSupportingWmCheck (Display* dpy, Window root);
	bool notGrabWindow(Window w) const { return w != grabWindow; }
	void createGrabWindow (Display* dpy, Window root, XSetWindowAttributes* attrib);
	void destroyGrabWindow (Display* dpy) { XDestroyWindow (dpy, grabWindow); }
	Time getCurrentTime (Display* dpy) const;
	Window const& getGrabWindow() const { return grabWindow; }
	void resetPossibleTap() { possibleTap = 0; }

    private:
        void *possibleTap;

	Glib::RefPtr <Glib::MainLoop>  mainloop;

	/* We cannot use RefPtrs. See
	 * https://bugzilla.gnome.org/show_bug.cgi?id=561885
	 */
	CompEventSource * source;
	CompTimeoutSource * timeout;
	CompSignalSource * sighupSource;
	CompSignalSource * sigtermSource;
	CompSignalSource * sigintSource;
	Glib::RefPtr <Glib::MainContext> ctx;

	CompFileWatchList   fileWatch;
	CompFileWatchHandle lastFileWatchHandle;

	// TODO - almost certainly the wrong data structure
	// Why not a std::map<CompWatchFdHandle, CompWatchFd>?
	std::list< CompWatchFd * > watchFds;
	CompWatchFdHandle        lastWatchFdHandle;

        bool	grabbed;   /* true once we receive a GrabNotify
			      on FocusOut and false on
			      UngrabNotify from FocusIn */
	Window  grabWindow;
};

class KeyGrab {
    public:
	int          keycode;
	unsigned int modifiers;
	int          count;
};

class ButtonGrab {
    public:
	int          button;
	unsigned int modifiers;
	int          count;
};

struct Grab {
	Grab(Cursor cursor, const char *name) : cursor(cursor), name(name) {}
	Cursor     cursor;
	const char *name;
};

// data members that don't belong (these probably belong
// in CompScreenImpl as PrivateScreen doesn't use them)
struct OrphanData : boost::noncopyable
{
    OrphanData();
    ~OrphanData();

    Window activeWindow;
    Window nextActiveWindow;
};

class GrabManager : boost::noncopyable
{
public:
    GrabManager(CompScreen *screen);

    bool addPassiveKeyGrab (CompAction::KeyBinding &key);
    void removePassiveKeyGrab (CompAction::KeyBinding &key);
    bool addPassiveButtonGrab (CompAction::ButtonBinding &button);
    void removePassiveButtonGrab (CompAction::ButtonBinding &button);

    void grabUngrabOneKey (unsigned int modifiers,
			   int          keycode,
			   bool         grab);
    bool grabUngrabKeys (unsigned int modifiers,
			 int          keycode,
			 bool         grab);
    void updatePassiveKeyGrabs ();
    void updatePassiveButtonGrabs(Window serverFrame);

private:
    CompScreen  * const screen;
    std::list<ButtonGrab> buttonGrabs;
    std::list<KeyGrab>    keyGrabs;
};

class History : public virtual ::compiz::History,
    boost::noncopyable
{
    public:
	History();

	void setCurrentActiveWindowHistory (int x, int y);

	void addToCurrentActiveWindowHistory (Window id);

	CompActiveWindowHistory* currentHistory ();

	unsigned int nextActiveNum () { return activeNum_++; }
	unsigned int activeNum () const;

    private:
	CompActiveWindowHistory history[ACTIVE_WINDOW_HISTORY_NUM];
	int                     currentHistory_;
	unsigned int activeNum_;
};

class ViewportRetrievalInterface
{
    public:

	virtual ~ViewportRetrievalInterface () {}

	virtual const CompPoint & getCurrentViewport () const = 0;
	virtual const CompSize & viewportDimentions () const = 0;
};

// Apart from a use by StartupSequence::addSequence this data
// is only used by CompScreenImpl - like the OrphanData struct
struct ViewPort :
    public ViewportRetrievalInterface
{
    public:

	ViewPort();
	CompPoint    vp;
	CompSize     vpSize;

    private:

	const CompPoint & getCurrentViewport () const { return vp; }
	const CompSize & viewportDimentions () const { return vpSize; }
};

namespace viewports
{
    void viewportForGeometry (const CompWindow::Geometry &gm,
			      CompPoint                   &viewport,
			      ViewportRetrievalInterface *viewports,
			      const CompSize &screenSize);
}

class StartupSequence : boost::noncopyable
{
    public:
	StartupSequence();
	void addSequence (SnStartupSequence *sequence, CompPoint const& vp);
	void removeSequence (SnStartupSequence *sequence);
	void removeAllSequences ();
	void applyStartupProperties (CompScreen* screen, CompWindow *window);
	bool handleStartupSequenceTimeout ();
	virtual void updateStartupFeedback () = 0;
	bool emptySequence() const { return startupSequences.empty(); }
    private:
	std::list<CompStartupSequence *> startupSequences;
	CompTimer                        startupSequenceTimer;
};

// Implemented as a separate class to break dependency on PrivateScreen & XWindows
class StartupSequenceImpl : public StartupSequence
{
    public:
	StartupSequenceImpl(PrivateScreen* priv) : priv(priv) {}

	virtual void updateStartupFeedback ();
    private:
	PrivateScreen* const priv;
};

class Extension
{
public:
    Extension() : is_enabled(), extension() {}

    template<Bool ExtensionQuery (Display*, int*, int*)>
    bool init(Display * dpy)
    {
	int error;
	is_enabled = ExtensionQuery(dpy, &extension, &error);
	return is_enabled;
    }

    template<Bool ExtensionQuery(Display*, int*, int*, int*, int*, int*)>
    bool init(Display * dpy)
    {
	int opcode;
	int error;
	is_enabled = ExtensionQuery(dpy, &opcode, &extension, &error, NULL, NULL);

	if (!is_enabled) extension = -1;

	return is_enabled;
    }

    int isEnabled () const { return is_enabled; }
    int get () const { return extension; }

private:
    bool is_enabled;
    int extension;
};

class Ping :
public virtual ::compiz::Ping
{
public:
    Ping() : lastPing_(1) {}
    bool handlePingTimeout (WindowManager::iterator begin, WindowManager::iterator end, Display* dpy);
    unsigned int lastPing () const { return lastPing_; }

private:
    unsigned int lastPing_;
};

class DesktopWindowCount :
    public virtual ::compiz::DesktopWindowCount
{
public:
    DesktopWindowCount();
    virtual void incrementDesktopWindowCount();
    virtual void decrementDesktopWindowCount();
    virtual int desktopWindowCount();
private:
    int       count;
};

class MapNum :
    public virtual ::compiz::MapNum
{
public:
    MapNum();
    virtual unsigned int nextMapNum();

private:
    unsigned int mapNum;
};

class XWindowInfo :
    public virtual ::compiz::XWindowInfo
{
public:
    XWindowInfo(Display* const& dpy) :
	dpy(dpy) {}

    virtual int getWmState (Window id);
    virtual void setWmState (int state, Window id) const;
    virtual void getMwmHints (Window id,
		      unsigned int *func,
		      unsigned int *decor) const;
    virtual unsigned int getProtocols (Window id);
    virtual unsigned int getWindowType (Window id);
    virtual unsigned int getWindowState (Window id);
private:
    Display* const& dpy;
};




unsigned int windowStateMask (Atom state);

}} // namespace compiz::private_screen

class FetchXEventInterface
{
    public:

	virtual ~FetchXEventInterface () {}

	virtual bool getNextXEvent (XEvent &) = 0;
};

class FetchEventInterface
{
    public:

	virtual ~FetchEventInterface () {}
	virtual bool getNextEvent (XEvent &) = 0;
};

class PrivateScreen :
    public CoreOptions,
    public FetchXEventInterface,
    public FetchEventInterface
{

    public:
	PrivateScreen (CompScreen *screen, compiz::private_screen::WindowManager& windowManager);
	~PrivateScreen ();

	bool initDisplay (
		const char *name,
		compiz::private_screen::History& history,
		unsigned int showingDesktopMask);

	bool setOption (const CompString &name, CompOption::Value &value);

	bool getNextEvent (XEvent &);
	bool getNextXEvent (XEvent &);
	void processEvents ();

	bool triggerButtonPressBindings (CompOption::Vector &options,
					 XButtonEvent       *event,
					 CompOption::Vector &arguments);

	bool triggerButtonReleaseBindings (CompOption::Vector &options,
					   XButtonEvent       *event,
					   CompOption::Vector &arguments);

	bool triggerKeyPressBindings (CompOption::Vector &options,
				      XKeyEvent          *event,
				      CompOption::Vector &arguments);

	bool triggerKeyReleaseBindings (CompOption::Vector &options,
					XKeyEvent          *event,
					CompOption::Vector &arguments);

	bool triggerStateNotifyBindings (CompOption::Vector  &options,
					 XkbStateNotifyEvent *event,
					 CompOption::Vector  &arguments);

	bool triggerEdgeEnter (unsigned int       edge,
			       CompAction::State  state,
			       CompOption::Vector &arguments);

	void setAudibleBell (bool audible);

	bool handleActionEvent (XEvent *event);

	void handleSelectionRequest (XEvent *event);

	void handleSelectionClear (XEvent *event);

	bool desktopHintEqual (unsigned long *data,
			       int           size,
			       int           offset,
			       int           hintSize);

	void setDesktopHints ();

	void setVirtualScreenSize (int hsize, int vsize);

	void updateScreenEdges ();

	void reshape (int w, int h);

	void getDesktopHints (unsigned int showingDesktopMask);

	void updateScreenInfo ();

	Window getActiveWindow (Window root);

	void setWindowState (unsigned int state, Window id);

	bool readWindowProp32 (Window         id,
			       Atom           property,
			       unsigned short *returnValue);

	void configure (XConfigureEvent *ce);

	void setNumberOfDesktops (unsigned int nDesktop);

	void setCurrentDesktop (unsigned int desktop);

	void enableEdge (int edge);

	void disableEdge (int edge);

	void setDefaultWindowAttributes (XWindowAttributes *);

	static void compScreenSnEvent (SnMonitorEvent *event,
			   void           *userData);

	int  getXkbEvent() const { return xkbEvent.get(); }
	std::vector<XineramaScreenInfo>& getScreenInfo () { return screenInfo; }
	SnDisplay* getSnDisplay () const { return snDisplay; }
	const char* displayString() const
    {
	return displayString_;
    }

    const CompRegion& getRegion() const
    {
	return region;
    }

    const XWindowAttributes& getAttrib() const
    {
	return attrib;
    }

    Window rootWindow() const
    {
	return root;
    }

    void identifyEdgeWindow(Window id);
    void setPlugins(const CompOption::Value::Vector& vList);
    void initPlugins();

    void updateClientList()
    {
	windowManager.updateClientList(*this);
    }

    void detectOutputDevices(CoreOptions& coreOptions);
    void updateOutputDevices(CoreOptions& coreOptions);

    void setPingTimerCallback(CompTimer::CallBack const& callback)
    { pingTimer.setCallback(callback); }

public:
    Display* dpy;
    compiz::private_screen::Extension xSync;
    compiz::private_screen::Extension xRandr;
    compiz::private_screen::Extension xShape;
    compiz::private_screen::ViewPort viewPort;
    compiz::private_screen::StartupSequenceImpl startupSequence;
    compiz::private_screen::EventManager eventManager;
    compiz::private_screen::OrphanData orphanData;
    compiz::private_screen::OutputDevices outputDevices;

    Colormap colormap;
    int screenNum;
    unsigned int nDesktop;
    unsigned int currentDesktop;

    CompOutput fullscreenOutput;
    CompScreenEdge screenEdge[SCREEN_EDGE_NUM];

    Window wmSnSelectionWindow;

    Cursor normalCursor;
    Cursor busyCursor;
    Cursor invisibleCursor;
    CompRect workArea;

    bool initialized;

private:
    CompScreen* screen;
    compiz::private_screen::Extension xkbEvent;

    //TODO? Pull these two out as a class?
    bool xineramaExtension;
    std::vector<XineramaScreenInfo> screenInfo;

    SnDisplay* snDisplay;
    char displayString_[256];
    KeyCode escapeKeyCode;
    KeyCode returnKeyCode;

    CompRegion region;
    Window root;
    XWindowAttributes attrib;

    SnMonitorContext* snContext;

    Atom wmSnAtom;
    Time wmSnTimestamp;

    unsigned long *desktopHintData;
    int desktopHintSize;

    Window edgeWindow;
    CompTimer pingTimer;
    CompTimer edgeDelayTimer;
    CompDelayedEdgeSettings edgeDelaySettings;
    Window xdndWindow;
    compiz::private_screen::PluginManager pluginManager;
    compiz::private_screen::WindowManager& windowManager;
};

class CompManager
{
    public:

	CompManager ();

	bool init ();
	void run ();
	void fini ();

	bool parseArguments (int, char **);
	void usage ();

	static bool initPlugin (CompPlugin *p);
	static void finiPlugin (CompPlugin *p);

    private:

	bool		       disableSm;
	char		       *clientId;
	char		       *displayName;
};

/**
 * A wrapping of the X display screen. This takes care of communication to the
 * X server.
 */
class CompScreenImpl : public CompScreen,
    public ServerGrabInterface,
    ::compiz::private_screen::DesktopWindowCount,
    ::compiz::private_screen::MapNum,
    ::compiz::private_screen::Ping,
    ::compiz::private_screen::XWindowInfo,
    ::compiz::private_screen::History
{
    public:
	CompScreenImpl ();
	~CompScreenImpl ();

	bool init (const char *name);

	void eventLoop ();

	CompFileWatchHandle addFileWatch (const char        *path,
					  int               mask,
					  FileWatchCallBack callBack);

	void removeFileWatch (CompFileWatchHandle handle);

	const CompFileWatchList& getFileWatches () const;

	CompWatchFdHandle addWatchFd (int             fd,
				      short int       events,
				      FdWatchCallBack callBack);

	void removeWatchFd (CompWatchFdHandle handle);

	void storeValue (CompString key, CompPrivate value);
	bool hasValue (CompString key);
	CompPrivate getValue (CompString key);
	void eraseValue (CompString key);

	Display * dpy ();

	CompOption::Vector & getOptions ();

	bool setOption (const CompString &name, CompOption::Value &value);

	bool XRandr ();

	int randrEvent ();

	bool XShape ();

	int shapeEvent ();

	int syncEvent ();

	SnDisplay * snDisplay ();

	Window activeWindow ();

	Window autoRaiseWindow ();

	const char * displayString ();


	CompWindow * findWindow (Window id);

	CompWindow * findTopLevelWindow (Window id,
					 bool   override_redirect = false);

	bool readImageFromFile (CompString &name,
				CompString &pname,
				CompSize   &size,
				void       *&data);

	bool writeImageToFile (CompString &path,
			       const char *format,
			       CompSize   &size,
			       void       *data);

	unsigned int getWindowProp (Window       id,
				    Atom         property,
				    unsigned int defaultValue);


	void setWindowProp (Window       id,
			    Atom         property,
			    unsigned int value);


	unsigned short getWindowProp32 (Window         id,
					Atom           property,
					unsigned short defaultValue);


	void setWindowProp32 (Window         id,
			      Atom           property,
			      unsigned short value);

	Window root ();

	int xkbEvent ();

	XWindowAttributes attrib ();

	int screenNum ();

	CompWindowList & windows ();
	CompWindowList & serverWindows ();
	CompWindowList & destroyedWindows ();

	void warpPointer (int dx, int dy);

	Time getCurrentTime ();

	Window selectionWindow ();

	void forEachWindow (CompWindow::ForEach);

	void focusDefaultWindow ();

	void insertWindow (CompWindow *w, Window aboveId);
	void unhookWindow (CompWindow *w);

	void insertServerWindow (CompWindow *w, Window aboveId);
	void unhookServerWindow (CompWindow *w);

	Cursor normalCursor ();

	Cursor invisibleCursor ();

	/* Adds an X Pointer and Keyboard grab to the stack. Since
	 * compiz as a client only need to grab once, multiple clients
	 * can call this and all get events, but the pointer will
	 * be grabbed once and the actual grab refcounted */
	GrabHandle pushGrab (Cursor cursor, const char *name);

	/* Allows you to change the pointer of your grab */
	void updateGrab (GrabHandle handle, Cursor cursor);

	/* Removes your grab from the stack. Once the internal refcount
	 * reaches zero, the X Pointer and Keyboard are both ungrabbed
	 */
	void removeGrab (GrabHandle handle, CompPoint *restorePointer);

	/* Returns true if a grab other than the grabs specified here
	 * exists */
	bool otherGrabExist (const char *, ...);

	/* Returns true if the specified grab exists */
	bool grabExist (const char *);

	/* Returns true if the X Pointer and / or Keyboard is grabbed
	 * by anything (another application, pluigins etc) */
	bool grabbed ();

	const CompWindowVector & clientList (bool stackingOrder);

	bool addAction (CompAction *action);

	void removeAction (CompAction *action);

	void updateWorkarea ();

	void toolkitAction (Atom   toolkitAction,
			    Time   eventTime,
			    Window window,
			    long   data0,
			    long   data1,
			    long   data2);

	void runCommand (CompString command);

	void moveViewport (int tx, int ty, bool sync);

	void sendWindowActivationRequest (Window id);

	int outputDeviceForPoint (int x, int y);
	int outputDeviceForPoint (const CompPoint &point);

	CompRect getCurrentOutputExtents ();

	const CompRect & getWorkareaForOutput (unsigned int outputNum) const;

	void viewportForGeometry (const CompWindow::Geometry &gm,
				  CompPoint                   &viewport);

	int outputDeviceForGeometry (const CompWindow::Geometry& gm);

	const CompPoint & vp () const;

	const CompSize  & vpSize () const;

	CompOutput::vector & outputDevs ();
	CompOutput & currentOutputDev () const;

	const CompRect & workArea () const;

	unsigned int currentDesktop ();

	unsigned int nDesktop ();

	bool shouldSerializePlugins () ;

	const CompRegion & region () const;

	bool hasOverlappingOutputs ();

	CompOutput & fullscreenOutput ();

	std::vector<XineramaScreenInfo> & screenInfo ();

	CompIcon *defaultIcon () const;

	bool updateDefaultIcon ();

	void updateSupportedWmHints ();

	unsigned int showingDesktopMask() const;
	virtual bool grabsEmpty() const;
	virtual void sizePluginClasses(unsigned int size);
	virtual void setWindowState (unsigned int state, Window id);
	virtual void addToDestroyedWindows(CompWindow * cw);
	virtual void processEvents ();
	virtual void alwaysHandleEvent (XEvent *event);

	virtual ServerGrabInterface * serverGrabInterface ();

	virtual void updatePassiveKeyGrabs () const;
	virtual void updatePassiveButtonGrabs(Window serverFrame);

	virtual bool displayInitialised() const;
	virtual void applyStartupProperties (CompWindow *window);
	virtual void updateClientList();
	virtual CompWindow * getTopWindow() const;
	virtual CompWindow * getTopServerWindow() const;
	virtual CoreOptions& getCoreOptions();
	virtual Colormap colormap() const;
	virtual void setCurrentDesktop (unsigned int desktop);
	virtual Window activeWindow() const;
	virtual bool grabWindowIsNot(Window w) const;
	virtual void incrementPendingDestroys();
	virtual void setNextActiveWindow(Window id);
	virtual Window getNextActiveWindow() const;
	virtual CompWindow * focusTopMostWindow ();

    public :

	static bool showDesktop (CompAction         *action,
				 CompAction::State  state,
				 CompOption::Vector &options);

	static bool windowMenu (CompAction         *action,
				CompAction::State  state,
				CompOption::Vector &options);

	static bool closeWin (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector &options);

	static bool unmaximizeWin (CompAction         *action,
				   CompAction::State  state,
				   CompOption::Vector &options);

	static bool minimizeWin (CompAction         *action,
				 CompAction::State  state,
				 CompOption::Vector &options);

	static bool maximizeWin (CompAction         *action,
				 CompAction::State  state,
				 CompOption::Vector &options);

	static bool maximizeWinHorizontally (CompAction         *action,
					     CompAction::State  state,
					     CompOption::Vector &options);

	static bool maximizeWinVertically (CompAction         *action,
					   CompAction::State  state,
					   CompOption::Vector &options);

	static bool raiseWin (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector &options);

	static bool lowerWin (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector &options);

	static bool toggleWinMaximized (CompAction         *action,
					CompAction::State  state,
					CompOption::Vector &options);

	static bool toggleWinMaximizedHorizontally (CompAction         *action,
						    CompAction::State  state,
						    CompOption::Vector &options);

	static bool toggleWinMaximizedVertically (CompAction         *action,
					          CompAction::State  state,
					          CompOption::Vector &options);

	static bool shadeWin (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector &options);

	bool createFailed () const;


    private:
        virtual bool _setOptionForPlugin(const char *, const char *, CompOption::Value &);
        virtual bool _initPluginForScreen(CompPlugin *);
        virtual void _finiPluginForScreen(CompPlugin *);
        virtual void _handleEvent(XEvent *event);
        virtual void _logMessage(const char *, CompLogLevel, const char*);
        virtual void _enterShowDesktopMode();
        virtual void _leaveShowDesktopMode(CompWindow *);
        virtual void _addSupportedAtoms(std::vector<Atom>& atoms);

        // These are stubs - but allow mocking of AbstractCompWindow
        virtual void _fileWatchAdded(CompFileWatch *);
        virtual void _fileWatchRemoved(CompFileWatch *);
        virtual void _sessionEvent(CompSession::Event, CompOption::Vector &);
        virtual void _handleCompizEvent(const char *, const char *, CompOption::Vector &);
        virtual bool _fileToImage(CompString &, CompSize &, int &, void *&);
        virtual bool _imageToFile(CompString &, CompString &, CompSize &, int, void *);
        virtual CompMatch::Expression * _matchInitExp(const CompString&);
        virtual void _matchExpHandlerChanged();
        virtual void _matchPropertyChanged(CompWindow *);
        virtual void _outputChangeNotify();

	void grabServer ();
	void ungrabServer ();
	void syncServer ();

        bool handlePingTimeout();

        Window below;
	CompTimer autoRaiseTimer_;
	Window    autoRaiseWindow_;
	CompIcon *defaultIcon_;
	compiz::private_screen::GrabManager mutable grabManager;
    	ValueHolder valueHolder;
        bool 	eventHandled;
        PrivateScreen privateScreen;
        compiz::private_screen::WindowManager windowManager;
        unsigned int showingDesktopMask_;
};

#endif
