/**
 *
 * Compiz Expo plugin
 *
 * click-threshold.cpp
 *
 * Copyright © 2012 Canonical Ltd.
 *
 * Authors:
 * Renato Araujo Oliviera Filho <renato@canonical.com>
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


#include "click-threshold.h"
#include <stdlib.h>

static const int DND_THRESHOLD = 5;

bool
compiz::expo::clickMovementInThreshold(int previousX, int previousY,
				       int currentX, int currentY)
{
    if ((abs (previousX - currentX) <= DND_THRESHOLD) &&
	(abs (previousY - currentY) <= DND_THRESHOLD))
	return true;
    else
	return false;
}
