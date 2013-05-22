/*
 * Copyright © 2012 Canonical, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical, Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical, Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL Canonical, Ltd. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#ifndef _COMPIZ_EVENT_MANAGEMENT_H
#define _COMPIZ_EVENT_MANAGEMENT_H

#include <boost/function.hpp>

struct CompScreenEdge;
class CompOption;

namespace compiz
{
namespace private_screen
{
class GrabList;
class EventManager;
}

namespace events
{
namespace cps = compiz::private_screen;
typedef std::vector <CompOption> EventArguments;
typedef boost::function <bool (unsigned int, unsigned int)> ActionModsMatchesEventStateFunc;

int
processButtonPressOnEdgeWindow (Window               edgeWindow,
				Window               root,
				Window               eventWindow,
				Window               eventRoot,
				cps::GrabList        &grabList,
				const CompScreenEdge *screenEdge);

void
setEventWindowInButtonPressArguments (EventArguments &arguments,
				      Window         eventWindow);

bool
activateButtonPressOnWindowBindingOption (CompOption                            &option,
					  unsigned int                          eventButton,
					  unsigned int                          eventState,
					  cps::EventManager                     &eventManager,
					  const ActionModsMatchesEventStateFunc &matchEventState,
					  EventArguments                        &arguments);

bool
activateButtonPressOnEdgeBindingOption (CompOption                            &option,
					unsigned int                          eventButton,
					unsigned int                          eventState,
					int                                   edge,
					cps::EventManager                     &eventManager,
					const ActionModsMatchesEventStateFunc &matchEventState,
					EventArguments                        &arguments);
}
}

	

#endif
