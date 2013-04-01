#include <test-pluginclasshandler.h>

class ConstructPlugin: public Plugin, public PluginClassHandler<ConstructPlugin,
	Base>
{
public:
    ConstructPlugin (Base * base) :
	    Plugin(base), PluginClassHandler<ConstructPlugin, Base>(base)
    {
    }
};

TEST_F( CompizPCHTest, TestConstruct )
{
    Plugin *p;

    bases.push_back(new Base());
    plugins.push_back(static_cast<Plugin *>(new ConstructPlugin(bases.back())));
    bases.push_back(new Base());
    plugins.push_back(static_cast<Plugin *>(new ConstructPlugin(bases.back())));

    if (bases.front()->pluginClasses.size() != globalPluginClassIndices.size())
    {
	FAIL() << "allocated number of plugin classes is not the same as the"
		" global number of allocated plugin classes";
    }

    if (!ValueHolder::Default()->hasValue(
	    compPrintf("%s_index_%lu", typeid(ConstructPlugin).name(), 0)))
    {
	FAIL() << "ValueHolder does not have value "
		<< compPrintf("%s_index_%lu", typeid(ConstructPlugin).name(), 0);
    }

    p = ConstructPlugin::get(bases.front());

    if (p != plugins.front())
    {
	FAIL() << "Returned Plugin * is not plugins.front ()";
    }

    p = ConstructPlugin::get(bases.back());

    if (p != plugins.back())
    {
	FAIL() << "Returned Plugin * is not plugins.back ()";
    }
}

