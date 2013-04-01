/*
 * Logging, Compiz configuration system library
 *
 * Copyright (C) 2012  Canonical Ltd.
 * Author: Daniel van Vugt <daniel.van.vugt@canonical.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "ccs.h"

/*
 * Only Info level or higher messages will be output by default. To see Debug
 * messages too, you must use:  env CCS_LOG_LEVEL=Debug ...
 */

static CCSLogLevel minLevel = _ccsLogLevels;

void
ccsLog (const char *domain, CCSLogLevel level, const char *fmt, ...)
{
    static const char * const levelName[_ccsLogLevels] = CCSLOGLEVEL_NAMES;

    if (minLevel == _ccsLogLevels)
    {
        char *env = getenv ("CCS_LOG_LEVEL");
        minLevel = ccsLogInfo;
        if (env != NULL)
        {
            int i = 0;
            while (i < _ccsLogLevels && strcmp (levelName[i], env))
                i++;
            if (i < _ccsLogLevels)
                minLevel = (CCSLogLevel)i;
        }
    }

    if (level >= minLevel && level < _ccsLogLevels)
    {
        FILE *file = (level >= ccsLogWarning) ? stderr : stdout;
        va_list va;

        va_start (va, fmt);

        /* I'm not a huge fan of this log format, but it matches compiz */
        fprintf (file, "compizconfig%s%s%s%s: ",
                 domain != NULL ? " ("   : " ",
                 domain != NULL ? domain : "",
                 domain != NULL ? ") - " : "- ",
                 levelName[level]);
        vfprintf (file, fmt, va);
        if (fmt[strlen(fmt)-1] != '\n')
            fprintf (file, "\n");
    
        va_end (va);
    }
}
