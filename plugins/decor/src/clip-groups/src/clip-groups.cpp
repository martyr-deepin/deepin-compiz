#include "clip-groups.h"
#include <boost/foreach.hpp>
#include <algorithm>

#ifndef foreach
#define foreach BOOST_FOREACH
#endif

using namespace compiz::decor;
using namespace compiz::decor::impl;

DecorClippableInterface::~DecorClippableInterface () {}
DecorClipGroupInterface::~DecorClipGroupInterface () {}

bool
GenericDecorClipGroup::doPushClippable (DecorClippableInterface *dc)
{
    std::vector <DecorClippableInterface *>::iterator it = std::find (mClippables.begin (),
								      mClippables.end (),
								      dc);

    if (it == mClippables.end ())
    {
	mClippables.push_back (dc);
	regenerateClipRegion ();
	dc->setOwner (this);

	return true;
    }

    return false;
}

bool
GenericDecorClipGroup::doPopClippable (DecorClippableInterface *dc)
{
    std::vector <DecorClippableInterface *>::iterator it = std::find (mClippables.begin (),
								      mClippables.end (),
								      dc);

    if (it != mClippables.end ())
    {
	dc->setOwner (NULL);
	dc->updateShadow (emptyRegion);
	mClippables.erase (it);
	regenerateClipRegion ();

	return true;
    }

    return false;
}

void
GenericDecorClipGroup::doRegenerateClipRegion ()
{
    mRegion -= infiniteRegion;

    foreach (DecorClippableInterface *clippable, mClippables)
    {
	mRegion += clippable->inputRegion ();
    }
}

const CompRegion &
GenericDecorClipGroup::getClipRegion ()
{
    return mRegion;
}

void
GenericDecorClipGroup::doUpdateAllShadows ()
{
    foreach (DecorClippableInterface *clippable, mClippables)
	clippable->updateShadow (mRegion);
}
