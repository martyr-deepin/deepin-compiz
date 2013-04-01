#include "test-pluginclasshandler.h"

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

PluginClassStorage::Indices globalPluginClassIndices (0);
unsigned int		    pluginClassHandlerIndex = 0;
bool			    debugOutput;
char			    *programName;

namespace
{
    static CompizPCHTest *gTest;
};

Global::Global ()
{
}

unsigned int
Base::allocPluginClassIndex ()
{
    unsigned int i = PluginClassStorage::allocatePluginClassIndex (globalPluginClassIndices);

    foreach (Base *b, gTest->global->bases)
    {
	if (globalPluginClassIndices.size () != b->pluginClasses.size ())
	    b->pluginClasses.resize (globalPluginClassIndices.size ());
    }

    return i;
}

void
Base::freePluginClassIndex (unsigned int index)
{
    PluginClassStorage::freePluginClassIndex (globalPluginClassIndices, index);

    foreach (Base *b, gTest->global->bases)
    {
	if (globalPluginClassIndices.size () != b->pluginClasses.size ())
	    b->pluginClasses.resize (globalPluginClassIndices.size ());
    }
}

Base::Base () :
    PluginClassStorage (globalPluginClassIndices)
{
    gTest->global->bases.push_back (this);
}

Base::~Base ()
{
    gTest->global->bases.remove (this);
}

Plugin::Plugin (Base *base) :
    b (base)
{
}

Plugin::~Plugin ()
{
}

CompizPCHTest::CompizPCHTest () :
    global (new Global())
{
    ValueHolder::SetDefault (static_cast<ValueHolder *> (global));
    gTest = this;
}

CompizPCHTest::~CompizPCHTest ()
{
    foreach (Plugin *p, plugins)
    {
	delete p;
    }

    foreach (Base *b, bases)
    {
	delete b;
    }

    delete global;
}
