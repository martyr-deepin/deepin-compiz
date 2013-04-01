#include <core/pluginclasshandler.h>
#include <core/pluginclasses.h>

#include <gtest/gtest.h>

#include <list>

extern PluginClassStorage::Indices globalPluginClassIndices;
extern unsigned int		    pluginClassHandlerIndex;
extern bool			    debugOutput;
extern char			    *programName;

class Base;

class Global:
    public ValueHolder
{
    public:

	Global ();

	std::list <Base *> bases;
};

class Base:
    public PluginClassStorage
{
    public:

	Base ();
	virtual ~Base ();

	static unsigned int allocPluginClassIndex ();
	static void freePluginClassIndex (unsigned int index);
};

class Plugin
{
    public:

	Plugin (Base *);
	virtual ~Plugin ();

	Base *b;
};

class CompizPCHTest : public ::testing::Test
{
public:

     CompizPCHTest ();
     virtual ~CompizPCHTest ();

     Global *global;
     std::list <Base *> bases;
     std::list <Plugin *> plugins;
};
