/**
 * Copyright © 2012 Canonical Ltd.
 *
 * Authors:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 **/
#ifndef _COMPIZ_EXPO_WINDOWS_ON_VIEWPORT_H
#define _COMPIZ_EXPO_WINDOWS_ON_VIEWPORT_H

#include <core/point.h>
#include <core/size.h>
#include <core/rect.h>
#include "client-list-generator.h"

namespace compiz
{
    namespace expo
    {
	unsigned int countViewports (const CompSize &vpSize);

	void fillInNewViewportActiveData (unsigned int vpCount,
					  std::vector <bool> &vpActive);

	void activeViewportsForMembers (compiz::expo::ClientListGenerator &clientList,
					const CompPoint                   &cursor,
					const CompSize			  &vpSize,
					const CompSize                    &screenSize,
					std::vector <bool>                &viewportActiveStates);
    }
}

#endif
