/*
 * Copyright (C) 2001 Havoc Pennington
 * Copyright (C) 2002, 2003 Red Hat, Inc.
 * Copyright (C) 2003 Rob Adams
 * Copyright (C) 2005 Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _COMPIZ_DECOR_CLIP_GROUPS_H
#define _COMPIZ_DECOR_CLIP_GROUPS_H

#include <vector>
#include <core/rect.h>
#include <core/region.h>
#include <core/match.h>

namespace compiz
{
    namespace decor
    {
	class DecorClipGroupInterface;

	class DecorClippableInterface
	{
	    public:

		virtual ~DecorClippableInterface () = 0;
		void updateShadow (const CompRegion &r) { doUpdateShadow (r); }
		void setOwner (DecorClipGroupInterface *i) { doSetOwner (i); }
		bool matches (const CompMatch &m) { return doMatches (m); }
		const CompRegion & outputRegion () { return getOutputRegion (); }
		const CompRegion & inputRegion () { return getInputRegion (); }
		void updateGroupShadows () { doUpdateGroupShadows (); }

	    private:

		virtual void doUpdateShadow (const CompRegion &) = 0;
		virtual void doSetOwner (DecorClipGroupInterface *i) = 0;
		virtual bool doMatches (const CompMatch &m) = 0;
		virtual const CompRegion & getOutputRegion () = 0;
		virtual const CompRegion & getInputRegion () = 0;
		virtual void doUpdateGroupShadows () = 0;
	};

	class DecorClipGroupInterface
	{
	    public:

		virtual ~DecorClipGroupInterface () = 0;

		bool pushClippable (DecorClippableInterface *dc) { return doPushClippable (dc); }
		bool popClippable (DecorClippableInterface *dc) { return doPopClippable (dc); }
		void regenerateClipRegion () { doRegenerateClipRegion (); }
		const CompRegion & clipRegion () { return getClipRegion (); }
		void updateAllShadows () { return doUpdateAllShadows (); }

	    private:

		virtual bool doPushClippable (DecorClippableInterface *dc) = 0;
		virtual bool doPopClippable (DecorClippableInterface *dc) = 0;
		virtual void doRegenerateClipRegion () = 0;
		virtual const CompRegion & getClipRegion () = 0;
		virtual void doUpdateAllShadows () = 0;
	};

	namespace impl
	{
	    class GenericDecorClipGroup :
		public DecorClipGroupInterface
	    {
		private:

		    bool doPushClippable (DecorClippableInterface *dc);
		    bool doPopClippable (DecorClippableInterface *dc);
		    void doRegenerateClipRegion ();
		    const CompRegion & getClipRegion ();
		    void doUpdateAllShadows ();

		    std::vector <DecorClippableInterface *> mClippables;
		    CompRegion                              mRegion;
	    };
	}
    }
}

#endif
