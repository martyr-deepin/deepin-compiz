#include <test-pluginclasshandler.h>

class GetPlugin :
    public Plugin,
    public PluginClassHandler <GetPlugin, Base>
{
    public:
	GetPlugin (Base *);
};

GetPlugin::GetPlugin (Base *base):
    Plugin (base),
    PluginClassHandler <GetPlugin, Base> (base)
{
}

TEST_F( CompizPCHTest, TestGet)
{
    Plugin *p;

    bases.push_back (new Base ());
    plugins.push_back (new GetPlugin (bases.back ()));
    bases.push_back (new Base ());
    plugins.push_back (new GetPlugin (bases.back ()));

    p = GetPlugin::get (bases.front ());

    if (p != plugins.front ())
    {
	FAIL() << "Returned Plugin * is not plugins.front ()";
    }

    p = GetPlugin::get (bases.back ());

    if (p != plugins.back ())
    {
	FAIL() << "Returned Plugin * is not plugins.back ()";
    }

    /* Now create a third base and check if plugin is implicitly created */

    bases.push_back (new Base ());
    plugins.push_back (GetPlugin::get (bases.back ()));

    p = plugins.back ();

    if (p->b != bases.back ())
    {
	FAIL() << "Returned Plugin * is not the plugin for bases.back ()";
    }
}
