/*
 * Compiz opengl plugin, Blacklist feature
 *
 * Copyright (c) 2012 Canonical Ltd.
 * Author: Daniel van Vugt <daniel.van.vugt@canonical.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "blacklist.h"
#include <cstdio>
#include <regex.h>

namespace compiz {
namespace opengl {

bool blacklisted (const char *blacklistRegex, const char *glVendor,
                  const char *glRenderer, const char *glVersion)
{
    bool matches = false;

    if (blacklistRegex && blacklistRegex[0])
    {
	regex_t re;

	// Ensure the regex contains something other than spaces, or ignore.
	const char *p = blacklistRegex;
	while (*p == ' ')
	    p++;

	if (*p && !regcomp (&re, blacklistRegex, REG_EXTENDED))
	{
	    char driver[1024];

	    snprintf (driver, sizeof driver, "%s\n%s\n%s",
	              glVendor ?   glVendor   : "",
	              glRenderer ? glRenderer : "",
	              glVersion ?  glVersion  : "");

	    if (!regexec (&re, driver, 0, NULL, 0))
		matches = true;

	    regfree (&re);
	}
    }

    return matches;
}

} // namespace opengl
} // namespace compiz
