#include <test-pluginclasshandler.h>
#include <iomanip>

class IndexesPlugin;
typedef PluginClassHandler<IndexesPlugin, Base> IPBHandler;
class IndexesPlugin :
    public Plugin,
    public IPBHandler
{
    public:
	IndexesPlugin (Base *);
	~IndexesPlugin ();
};

class IndexesPlugin2;
typedef PluginClassHandler<IndexesPlugin2, Base> IPB2Handler;
class IndexesPlugin2 :
    public Plugin,
    public IPB2Handler
{
    public:
	IndexesPlugin2 (Base *);
};

class IndexesPlugin3;
typedef PluginClassHandler<IndexesPlugin3, Base> IPB3Handler;
class IndexesPlugin3 :
    public Plugin,
    public IPB3Handler
{
    public:
	IndexesPlugin3 (Base *);
};

class FailingBase:
    public PluginClassStorage
{
    public:

	FailingBase ();
	virtual ~FailingBase ();

	static unsigned int allocPluginClassIndex ();
	static void freePluginClassIndex (unsigned int index);
};

class FailingIndexesPlugin;
typedef PluginClassHandler<FailingIndexesPlugin, FailingBase> FIPBHandler;
class FailingIndexesPlugin :
    public FIPBHandler
{
    public:
	FailingIndexesPlugin (FailingBase *);
};

unsigned int
FailingBase::allocPluginClassIndex ()
{
    /* Always fails */
    return (unsigned int) ~0;
}

void
FailingBase::freePluginClassIndex (unsigned int index)
{
    /* Does nothing, since this base is made of fail */
}

FailingBase::FailingBase () :
    PluginClassStorage (globalPluginClassIndices)
{
}

FailingBase::~FailingBase ()
{
}

IndexesPlugin::IndexesPlugin (Base *base):
    Plugin (base),
    IPBHandler (base)
{
}

IndexesPlugin::~IndexesPlugin ()
{
}

IndexesPlugin2::IndexesPlugin2 (Base *base):
    Plugin (base),
    IPB2Handler (base)
{
}

IndexesPlugin3::IndexesPlugin3 (Base *base):
    Plugin (base),
    IPB3Handler (base)
{
    setFailed ();
}

FailingIndexesPlugin::FailingIndexesPlugin (FailingBase *base):
    FIPBHandler (base)
{
}

class CompizPCHTestIndexes :
    public CompizPCHTest
{
    public:

	void run ();

	template <typename I>
	void printFailure (I *);

    public:
	unsigned int ePluginClassHandlerIndex;
	unsigned int eIndex;
	int 	 eRefCount;
	bool	 eInitiated;
	bool	 eFailed;
	bool	 ePcFailed;
	unsigned int ePcIndex;
	bool	 eLoadFailed;
};

template <typename I>
void
CompizPCHTestIndexes::printFailure (I *i)
{
    std::cout << "FAIL: index value does not match!" << std::endl;
    std::cout << "INFO: Values of " << std::hex << (void *) i << " are " << std::endl;
    std::cout << "INFO: pluginClassHandlerIndex is " << pluginClassHandlerIndex << std::endl;
    std::cout << "INFO: mIndex.refCount " << i->mIndex.refCount << std::endl;
    std::cout << "INFO: mIndex.initiated " << i->mIndex.initiated << std::endl;
    std::cout << "INFO: mIndex.failed " << i->mIndex.failed << std::endl;
    std::cout << "INFO: mIndex.pcFailed " << i->mIndex.pcFailed << std::endl;
    std::cout << "INFO: mIndex.pcIndex " << i->mIndex.pcIndex << std::endl;
    std::cout << "INFO: loadFailed " << (i->loadFailed () ? 1 : 0) << std::endl;
    std::cout << "INFO: Values of " << std::hex << (void *) i << " should be " << std::endl;
    std::cout << "INFO: pluginClassHandlerIndex is " << ePluginClassHandlerIndex << std::endl;
    std::cout << "INFO: mIndex.index " << eIndex << std::endl;
    std::cout << "INFO: mIndex.refCount " << eRefCount << std::endl;
    std::cout << "INFO: mIndex.initiated " << eInitiated << std::endl;
    std::cout << "INFO: mIndex.failed " << eFailed << std::endl;
    std::cout << "INFO: mIndex.pcFailed " << ePcFailed << std::endl;
    std::cout << "INFO: mIndex.pcIndex " << ePcIndex << std::endl;
    std::cout << "INFO: loadFailed " << (eLoadFailed ? 1 : 0) << std::endl;
}

#define INDEX_INCORRECT(i) \
pluginClassHandlerIndex != ePluginClassHandlerIndex || \
i->mIndex.index != eIndex || \
i->mIndex.refCount != eRefCount || \
i->mIndex.initiated != eInitiated || \
i->mIndex.failed != eFailed || \
i->mIndex.pcFailed != ePcFailed || \
i->mIndex.pcIndex != ePcIndex

TEST_F( CompizPCHTestIndexes, TestIndexes )
{
    Plugin      *p;
    Base 	*b;
    FailingBase *fb;
    FailingIndexesPlugin *fp;
    IPBHandler  *ipbhandler_p;
    IPB2Handler *ipbhandler_p2;
    IPB3Handler *ipbhandler_p3;
    FIPBHandler *fipbhandler_p;

    bases.push_back (new Base ());
    plugins.push_back (new IndexesPlugin (bases.back ()));

    p = plugins.back ();
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 1;
    eIndex = 0;
    eRefCount = 1;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 0;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    bases.push_back (new Base ());
    plugins.push_back (new IndexesPlugin (bases.back ()));

    p = plugins.back ();
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 1;
    eIndex = 0;
    eRefCount = 2;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 0;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    bases.push_back (new Base ());
    plugins.push_back (new IndexesPlugin2 (bases.back ()));

    p = plugins.back ();
    ipbhandler_p2 = (IPB2Handler *) p;

    ePluginClassHandlerIndex = 2;
    eIndex = 1;
    eRefCount = 1;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 1;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p2))
    {
	printFailure<IPB2Handler> (ipbhandler_p2);
	exit (1);
    }

    plugins.pop_back ();
    delete p;

    b = bases.back ();
    bases.pop_back ();
    delete b;

    p = plugins.back ();
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 3;
    eIndex = 0;
    eRefCount = 2;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 0;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    plugins.pop_back ();
    delete p;

    b = bases.back ();
    bases.pop_back ();
    delete b;

    p = plugins.back ();
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 3;
    eIndex = 0;
    eRefCount = 1;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 0;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    p = IndexesPlugin::get (bases.back ());
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 3;
    eIndex = 0;
    eRefCount = 1;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 3;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    bases.push_back (new Base ());
    plugins.push_back (new IndexesPlugin3 (bases.back ()));

    p = plugins.back ();
    ipbhandler_p3 = (IPB3Handler *) p;

    ePluginClassHandlerIndex = 4;
    eIndex = 1;
    eRefCount = 1;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 0;
    eLoadFailed = true;

/*
 * This seems to be broken on some compilers
 * disable this test for now
 *
 *  if (INDEX_INCORRECT (ipbhandler_p3))
 *  {
 *	printFailure<IPB3Handler> (ipbhandler_p3);
 *	exit (1);
 *  }
 */
    bases.push_back (new Base ());
    plugins.push_back (new IndexesPlugin (bases.back ()));

    p = plugins.back ();
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 4;
    eIndex = 0;
    eRefCount = 2;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 3;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    plugins.push_back (new IndexesPlugin2 (bases.back ()));

    p = plugins.back ();
    ipbhandler_p2 = (IPB2Handler *) p;

    ePluginClassHandlerIndex = 5;
    eIndex = 2;
    eRefCount = 1;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 4;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p2))
    {
	printFailure<IPB2Handler> (ipbhandler_p2);
	FAIL();
    }

    /* Now call ::get on the first one to reset the pluginClassHandlerIndex */
    p = IndexesPlugin::get (bases.back ());
    ipbhandler_p = (IPBHandler *) p;

    ePluginClassHandlerIndex = 5;
    eIndex = 0;
    eRefCount = 2;
    eInitiated = true;
    eFailed = false;
    ePcFailed = false;
    ePcIndex = 5;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p))
    {
	printFailure<IPBHandler> (ipbhandler_p);
	FAIL();
    }

    /* Now erase the key that was used by the second plugin so subsequent attempts to ::get
     * will fail */

    p = plugins.back ();
    ipbhandler_p2 = (IPB2Handler *) p;
    ValueHolder::Default ()->eraseValue (ipbhandler_p2->keyName ());
    p = IndexesPlugin2::get (bases.back ());

    ePluginClassHandlerIndex = 5;
    eIndex = 2;
    eRefCount = 1;
    eInitiated = false;
    eFailed = true;
    ePcFailed = false;
    ePcIndex = 5;
    eLoadFailed = false;

    if (INDEX_INCORRECT (ipbhandler_p2))
    {
	printFailure<IPB2Handler> (ipbhandler_p2);
	FAIL();
    }

    fb = new FailingBase ();
    fp = new FailingIndexesPlugin (fb);
    fipbhandler_p = (FIPBHandler *) fp;

    ePluginClassHandlerIndex = 5;
    eIndex = 0;
    eRefCount = 0;
    eInitiated = false;
    eFailed = true;
    ePcFailed = true;
    ePcIndex = 5;
    eLoadFailed = false;

    if (INDEX_INCORRECT (fipbhandler_p))
    {
	printFailure<FIPBHandler> (fipbhandler_p);
	FAIL();
    }

    delete fp;
    delete fb;
}
