/*
 * Copyright © 2009 Danny Baumann
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Danny Baumann not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Danny Baumann makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DANNY BAUMANN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Danny Baumann <dannybaumann@web.de>
 */

#include "commands.h"

#include "core/abiversion.h"
#include "core/screen.h"


COMPIZ_PLUGIN_20090315 (commands, CommandsPluginVTable);

bool
CommandsScreen::runCommand (CompAction          *action,
			    CompAction::State   state,
			    CompOption::Vector& options,
			    int                 commandOption)
{
    CommandsScreen *cs;
    Window         xid;

    xid = CompOption::getIntOptionNamed (options, "root", 0);
    if (xid != screen->root ())
	return false;

    cs = CommandsScreen::get (screen);

    screen->runCommand (cs->mOptions[commandOption].value (). s ());

    return true;
}


CommandsScreen::CommandsScreen (CompScreen *s) :
    PluginClassHandler<CommandsScreen, CompScreen> (s)
{

#define DISPATCH(opt) boost::bind (CommandsScreen::runCommand, _1, _2, _3, opt)

    optionSetRunCommand0KeyInitiate (DISPATCH (CommandsOptions::Command0));
    optionSetRunCommand1KeyInitiate (DISPATCH (CommandsOptions::Command1));
    optionSetRunCommand2KeyInitiate (DISPATCH (CommandsOptions::Command2));
    optionSetRunCommand3KeyInitiate (DISPATCH (CommandsOptions::Command3));
    optionSetRunCommand4KeyInitiate (DISPATCH (CommandsOptions::Command4));
    optionSetRunCommand5KeyInitiate (DISPATCH (CommandsOptions::Command5));
    optionSetRunCommand6KeyInitiate (DISPATCH (CommandsOptions::Command6));
    optionSetRunCommand7KeyInitiate (DISPATCH (CommandsOptions::Command7));
    optionSetRunCommand8KeyInitiate (DISPATCH (CommandsOptions::Command8));
    optionSetRunCommand9KeyInitiate (DISPATCH (CommandsOptions::Command9));
    optionSetRunCommand10KeyInitiate (DISPATCH (CommandsOptions::Command10));
    optionSetRunCommand11KeyInitiate (DISPATCH (CommandsOptions::Command11));
    optionSetRunCommand12KeyInitiate (DISPATCH (CommandsOptions::Command12));
    optionSetRunCommand13KeyInitiate (DISPATCH (CommandsOptions::Command13));
    optionSetRunCommand14KeyInitiate (DISPATCH (CommandsOptions::Command14));
    optionSetRunCommand15KeyInitiate (DISPATCH (CommandsOptions::Command15));
    optionSetRunCommand16KeyInitiate (DISPATCH (CommandsOptions::Command16));
    optionSetRunCommand17KeyInitiate (DISPATCH (CommandsOptions::Command17));
    optionSetRunCommand18KeyInitiate (DISPATCH (CommandsOptions::Command18));
    optionSetRunCommand19KeyInitiate (DISPATCH (CommandsOptions::Command19));
    optionSetRunCommand20KeyInitiate (DISPATCH (CommandsOptions::Command20));

    optionSetRunCommand0ButtonInitiate (DISPATCH (CommandsOptions::Command0));
    optionSetRunCommand1ButtonInitiate (DISPATCH (CommandsOptions::Command1));
    optionSetRunCommand2ButtonInitiate (DISPATCH (CommandsOptions::Command2));
    optionSetRunCommand3ButtonInitiate (DISPATCH (CommandsOptions::Command3));
    optionSetRunCommand4ButtonInitiate (DISPATCH (CommandsOptions::Command4));
    optionSetRunCommand5ButtonInitiate (DISPATCH (CommandsOptions::Command5));
    optionSetRunCommand6ButtonInitiate (DISPATCH (CommandsOptions::Command6));
    optionSetRunCommand7ButtonInitiate (DISPATCH (CommandsOptions::Command7));
    optionSetRunCommand8ButtonInitiate (DISPATCH (CommandsOptions::Command8));
    optionSetRunCommand9ButtonInitiate (DISPATCH (CommandsOptions::Command9));
    optionSetRunCommand10ButtonInitiate (DISPATCH (CommandsOptions::Command10));
    optionSetRunCommand11ButtonInitiate (DISPATCH (CommandsOptions::Command11));
    optionSetRunCommand12ButtonInitiate (DISPATCH (CommandsOptions::Command12));
    optionSetRunCommand13ButtonInitiate (DISPATCH (CommandsOptions::Command13));
    optionSetRunCommand14ButtonInitiate (DISPATCH (CommandsOptions::Command14));
    optionSetRunCommand15ButtonInitiate (DISPATCH (CommandsOptions::Command15));
    optionSetRunCommand16ButtonInitiate (DISPATCH (CommandsOptions::Command16));
    optionSetRunCommand17ButtonInitiate (DISPATCH (CommandsOptions::Command17));
    optionSetRunCommand18ButtonInitiate (DISPATCH (CommandsOptions::Command18));
    optionSetRunCommand19ButtonInitiate (DISPATCH (CommandsOptions::Command19));
    optionSetRunCommand20ButtonInitiate (DISPATCH (CommandsOptions::Command20));

    optionSetRunCommand0EdgeInitiate (DISPATCH (CommandsOptions::Command0));
    optionSetRunCommand1EdgeInitiate (DISPATCH (CommandsOptions::Command1));
    optionSetRunCommand2EdgeInitiate (DISPATCH (CommandsOptions::Command2));
    optionSetRunCommand3EdgeInitiate (DISPATCH (CommandsOptions::Command3));
    optionSetRunCommand4EdgeInitiate (DISPATCH (CommandsOptions::Command4));
    optionSetRunCommand5EdgeInitiate (DISPATCH (CommandsOptions::Command5));
    optionSetRunCommand6EdgeInitiate (DISPATCH (CommandsOptions::Command6));
    optionSetRunCommand7EdgeInitiate (DISPATCH (CommandsOptions::Command7));
    optionSetRunCommand8EdgeInitiate (DISPATCH (CommandsOptions::Command8));
    optionSetRunCommand9EdgeInitiate (DISPATCH (CommandsOptions::Command9));
    optionSetRunCommand10EdgeInitiate (DISPATCH (CommandsOptions::Command10));
    optionSetRunCommand11EdgeInitiate (DISPATCH (CommandsOptions::Command11));
    optionSetRunCommand12EdgeInitiate (DISPATCH (CommandsOptions::Command12));
    optionSetRunCommand13EdgeInitiate (DISPATCH (CommandsOptions::Command13));
    optionSetRunCommand14EdgeInitiate (DISPATCH (CommandsOptions::Command14));
    optionSetRunCommand15EdgeInitiate (DISPATCH (CommandsOptions::Command15));
    optionSetRunCommand16EdgeInitiate (DISPATCH (CommandsOptions::Command16));
    optionSetRunCommand17EdgeInitiate (DISPATCH (CommandsOptions::Command17));
    optionSetRunCommand18EdgeInitiate (DISPATCH (CommandsOptions::Command18));
    optionSetRunCommand19EdgeInitiate (DISPATCH (CommandsOptions::Command19));
    optionSetRunCommand20EdgeInitiate (DISPATCH (CommandsOptions::Command20));
}

bool
CommandsPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}

