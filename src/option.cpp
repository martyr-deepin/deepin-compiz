/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <core/option.h>
#include "privateoption.h"
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/type_traits/remove_const.hpp>

namespace
{
    CompOption::Value::Vector & emptyList ()
    {
	static CompOption::Value::Vector v;
	return v;
    }

    CompString & emptyString ()
    {
	static CompString v;
	return v;
    }

    CompMatch & emptyMatch ()
    {
	static CompMatch v;
	return v;
    }

    CompAction & emptyAction ()
    {
	static CompAction v;
	return v;
    }

    unsigned short * emptyColor ()
    {
	static unsigned short v[4] = { 0, 0, 0, 0 };
	return v;
    }


    template<typename TargetType, typename boost::remove_const<TargetType>::type & (*get_default)()>
    struct get_or_default_call : public boost::static_visitor<TargetType &>
    {
	TargetType& operator()(TargetType& a)
	{
	    return a;
	}

	template<typename T>
	TargetType& operator()(T&)
	{
	    return get_default();
	}
    };

    template<typename TargetType, typename boost::remove_const<TargetType>::type Default>
    struct get_or_default_val : public boost::static_visitor<TargetType>
    {
	TargetType operator()(TargetType& a)
	{
	    return a;
	}

	template<typename T>
	TargetType operator()(T&)
	{
	    return Default;
	}
    };

    struct get_float : public boost::static_visitor<float const>
    {
	float operator()(float const& a)
	{
	    return a;
	}

	template<typename T>
	float operator()(T&)
	{
	    return 0.0;
	}
    };
}

CompOption::Vector &
noOptions ()
{
    static CompOption::Vector v;
    return v;
}



static bool
checkIsAction (CompOption::Type type)
{
    switch (type) {
	case CompOption::TypeAction:
	case CompOption::TypeKey:
	case CompOption::TypeButton:
	case CompOption::TypeEdge:
	case CompOption::TypeBell:
	    return true;
	default:
	    break;
    }

    return false;
}

CompOption::Value::~Value()
{
}

void
CompOption::Value::set (Type t, const CompOption::Value::Vector & v)
{
    mListType = t;
    mValue = v;
}

bool
CompOption::Value::b () const
{
    get_or_default_val<bool const, false> tmp;
    return boost::apply_visitor(tmp, mValue);
}

int
CompOption::Value::i () const
{
    get_or_default_val<int const, 0> tmp;
    return boost::apply_visitor(tmp, mValue);
}

float
CompOption::Value::f () const
{
    get_float tmp;
    return boost::apply_visitor(tmp, mValue);
}

unsigned short*
CompOption::Value::c () const
{
    try
    {
	return const_cast <unsigned short *> (&((boost::get<ColorVector>(mValue))[0]));
    }
    catch (...)
    {
	return emptyColor ();
    }
}

const CompString &
CompOption::Value::s () const
{
    get_or_default_call<CompString const, emptyString> tmp;
    return boost::apply_visitor(tmp, mValue);
}

CompString &
CompOption::Value::s ()
{
    get_or_default_call<CompString, emptyString> tmp;
    return boost::apply_visitor(tmp, mValue);
}

const CompMatch &
CompOption::Value::match () const
{
    get_or_default_call<CompMatch const, emptyMatch> tmp;
    return boost::apply_visitor(tmp, mValue);
}

CompMatch &
CompOption::Value::match ()
{
    get_or_default_call<CompMatch, emptyMatch> tmp;
    return boost::apply_visitor(tmp, mValue);
}

const CompAction &
CompOption::Value::action () const
{
    get_or_default_call<CompAction const, emptyAction> tmp;
    return boost::apply_visitor(tmp, mValue);
}

CompAction &
CompOption::Value::action ()
{
    get_or_default_call<CompAction, emptyAction> tmp;
    return boost::apply_visitor(tmp, mValue);
}

// Type listType () const;

const CompOption::Value::Vector &
CompOption::Value::list () const
{
    get_or_default_call<CompOption::Value::Vector const, emptyList> tmp;
    return boost::apply_visitor(tmp, mValue);
}

CompOption::Value::Vector &
CompOption::Value::list ()
{
    get_or_default_call<CompOption::Value::Vector, emptyList> tmp;
    return boost::apply_visitor(tmp, mValue);
}

bool
CompOption::Value::operator== (const Value & rhs) const
{
    return mValue == rhs.mValue;
}

bool
CompOption::Value::operator!= (const Value & rhs) const
{
    return !(mValue == rhs.mValue);
}

CompOption::Restriction::Restriction () :
    priv (new PrivateRestriction ())
{
}

CompOption::Restriction::Restriction (const CompOption::Restriction &r) :
    priv (new PrivateRestriction (*r.priv))
{
}

CompOption::Restriction::~Restriction ()
{
    delete priv;
}

int
CompOption::Restriction::iMin ()
{
    if (priv->type == CompOption::TypeInt)
	return priv->rest.i.min;
    return MINSHORT;
}

int
CompOption::Restriction::iMax ()
{
    if (priv->type == CompOption::TypeInt)
	return priv->rest.i.max;
    return MAXSHORT;
}

float
CompOption::Restriction::fMin ()
{
    if (priv->type == CompOption::TypeFloat)
	return priv->rest.f.min;
    return MINSHORT;
}

float
CompOption::Restriction::fMax ()
{
    if (priv->type == CompOption::TypeFloat)
	return priv->rest.f.min;
    return MINSHORT;
}

float
CompOption::Restriction::fPrecision ()
{
    if (priv->type == CompOption::TypeFloat)
	return priv->rest.f.precision;
    return 0.1f;
}


void
CompOption::Restriction::set (int min, int max)
{
    priv->type = CompOption::TypeInt;
    priv->rest.i.min = min;
    priv->rest.i.max = max;
}

void
CompOption::Restriction::set (float min, float max, float precision)
{
    priv->type = CompOption::TypeFloat;
    priv->rest.f.min       = min;
    priv->rest.f.max       = max;
    priv->rest.f.precision = precision;
}

bool
CompOption::Restriction::inRange (int i)
{
    if (priv->type != CompOption::TypeInt)
	return true;
    if (i < priv->rest.i.min)
	return false;
    if (i > priv->rest.i.max)
	return false;
    return true;
}

bool
CompOption::Restriction::inRange (float f)
{
    if (priv->type != CompOption::TypeFloat)
	return true;
    if (f < priv->rest.f.min)
	return false;
    if (f > priv->rest.f.max)
	return false;
    return true;
}

CompOption::Restriction &
CompOption::Restriction::operator= (const CompOption::Restriction &rest)
{
    if (this == &rest)
	return *this;

    delete priv;
    priv = new PrivateRestriction (*rest.priv);
    return *this;
}

CompOption *
CompOption::Class::getOption (const CompString &name)
{
    CompOption *o = CompOption::findOption (getOptions (), name);
    return o;
}

CompOption *
CompOption::findOption (CompOption::Vector &options,
			CompString         name,
			unsigned int       *index)
{
    unsigned int i;

    for (i = 0; i < options.size (); i++)
    {
	if (options[i].priv->name == name)
	{
	    if (index)
		*index = i;

	    return &options[i];
	}
    }

    return NULL;
}

CompOption::CompOption () :
    priv (new PrivateOption ())
{
}

CompOption::CompOption (const CompOption &o) :
    priv (new PrivateOption (*o.priv))
{
}

CompOption::CompOption (CompString name, CompOption::Type type) :
    priv (new PrivateOption ())
{
    setName (name, type);
}

CompOption::~CompOption ()
{
    /* Remove any added actions */
    try
    {
	CompAction &action = value ().action ();

	if (action.active () && screen)
	    screen->removeAction (&action);
    }
    catch (...)
    {
    }

    delete priv;
}

void
CompOption::reset ()
{
    priv->name = "";
    priv->type = TypeUnset;
}

void
CompOption::setName (CompString name, CompOption::Type type)
{
    priv->name = name;
    priv->type = type;
}

void
CompOption::setName (const char *name, CompOption::Type type)
{
    if (!name && !priv->name.empty ())
	priv->name.clear ();
    else if (name && priv->name != name)
	priv->name = name;
    priv->type = type;
}

CompString
CompOption::name ()
{
    return priv->name;
}

CompOption::Type
CompOption::type ()
{
    return priv->type;
}

CompOption::Value &
CompOption::value ()
{
    return priv->value;
}

CompOption::Restriction &
CompOption::rest ()
{
    return priv->rest;
}

bool
CompOption::set (CompOption::Value &val)
{
    if (isAction () && priv->type != CompOption::TypeAction)
	val.action ().copyState (priv->value.action ());

    if (priv->type != val.type () &&
	(!isAction () || !checkIsAction (val.type ())))
    {
	compLogMessage ("core", CompLogLevelWarn,
			"Can't set Value with type %d to "
			"option \"%s\" with type %d",
			val.type (), priv->name.c_str (), priv->type);
	return false;
    }

    if (priv->value == val)
	return false;

    if (isAction () &&
        priv->value.action ().state () & CompAction::StateAutoGrab && screen)
    {
	if (!screen->addAction (&val.action ()))
	    return false;
	else
	    screen->removeAction (&priv->value.action ());
    }

    switch (priv->type)
    {
	case CompOption::TypeInt:
	    if (!priv->rest.inRange (val.i ()))
		return false;
	    break;

	case CompOption::TypeFloat:
	{
	    float v, p;
	    int sign = (val.f () < 0 ? -1 : 1);

	    if (!priv->rest.inRange (val.f ()))
		return false;

	    p = 1.0f / priv->rest.fPrecision ();
	    v = ((int) (val.f () * p + sign * 0.5f)) / p;

	    priv->value.set (v);
	    return true;
	}

	case CompOption::TypeAction:
	    return false;

	case CompOption::TypeKey:
	    if (val.action ().type () == value().action ().type () &&
		!(val.action ().type () & CompAction::BindingTypeKey))
		return false;
	    break;

	case CompOption::TypeButton:
	    if (val.action ().type () == value().action ().type () &&
		!(val.action ().type () & (CompAction::BindingTypeButton |
					   CompAction::BindingTypeEdgeButton)))
		return false;
	    break;

	default:
	    break;
    }

    priv->value = val;

    return true;
}

bool
CompOption::isAction ()
{
    return checkIsAction (priv->type);
}

CompOption &
CompOption::operator= (const CompOption &option)
{
    if (this == &option)
	return *this;

    delete priv;
    priv = new PrivateOption (*option.priv);
    return *this;
}

bool
CompOption::getBoolOptionNamed (const Vector&     options,
				const CompString& name,
				bool              defaultValue)
{
    foreach (const CompOption &o, options)
	if (o.priv->type == CompOption::TypeBool && o.priv->name == name)
	    return o.priv->value.b ();

    return defaultValue;
}

int
CompOption::getIntOptionNamed (const Vector&     options,
			       const CompString& name,
			       int               defaultValue)
{
    foreach (const CompOption &o, options)
	if (o.priv->type == CompOption::TypeInt && o.priv->name == name)
	    return o.priv->value.i ();

    return defaultValue;
}

float
CompOption::getFloatOptionNamed (const Vector&     options,
				 const CompString& name,
				 const float&      defaultValue)
{
    foreach (const CompOption &o, options)
	if (o.priv->type == CompOption::TypeFloat && o.priv->name == name)
	    return o.priv->value.f ();

    return defaultValue;
}

CompString
CompOption::getStringOptionNamed (const Vector&     options,
				  const CompString& name,
				  const CompString& defaultValue)
{
    foreach (const CompOption &o, options)
	if (o.priv->type == CompOption::TypeString && o.priv->name == name)
	    return o.priv->value.s ();

    return defaultValue;
}

unsigned short *
CompOption::getColorOptionNamed (const Vector&        options,
				 const CompString&    name,
				 unsigned short       *defaultValue)
{
    foreach (const CompOption &o, options)
	if (o.priv->type == CompOption::TypeColor && o.priv->name == name)
	    return o.priv->value.c ();

    return defaultValue;
}

CompMatch
CompOption::getMatchOptionNamed (const Vector&     options,
				 const CompString& name,
				 const CompMatch&  defaultValue)
{
    foreach (const CompOption &o, options)
	if (o.priv->type == CompOption::TypeMatch && o.priv->name == name)
	    return o.priv->value.match ();

    return defaultValue;
}

bool
CompOption::stringToColor (CompString     color,
			   unsigned short *rgba)
{
    int c[4];

    if (sscanf (color.c_str (), "#%2x%2x%2x%2x",
		&c[0], &c[1], &c[2], &c[3]) == 4)
    {
	rgba[0] = c[0] << 8 | c[0];
	rgba[1] = c[1] << 8 | c[1];
	rgba[2] = c[2] << 8 | c[2];
	rgba[3] = c[3] << 8 | c[3];

	return true;
    }

    return false;
}

CompString
CompOption::colorToString (unsigned short *rgba)
{
    return compPrintf ("#%.2x%.2x%.2x%.2x", rgba[0] / 256, rgba[1] / 256,
					    rgba[2] / 256, rgba[3] / 256);
}

CompString
CompOption::typeToString (CompOption::Type type)
{
    switch (type) {
	case CompOption::TypeBool:
	    return "bool";
	case CompOption::TypeInt:
	    return "int";
	case CompOption::TypeFloat:
	    return "float";
	case CompOption::TypeString:
	    return "string";
	case CompOption::TypeColor:
	    return "color";
	case CompOption::TypeAction:
	    return "action";
	case CompOption::TypeKey:
	    return "key";
	case CompOption::TypeButton:
	    return "button";
	case CompOption::TypeEdge:
	    return "edge";
	case CompOption::TypeBell:
	    return "bell";
	case CompOption::TypeMatch:
	    return "match";
	case CompOption::TypeList:
	    return "list";
	default:
	    break;
    }

    return "unknown";
}

bool
CompOption::setOption (CompOption        &o,
		       CompOption::Value &value)
{
    return o.set (value);
}

PrivateOption::PrivateOption () :
    name (""),
    type (CompOption::TypeUnset),
    value (),
    rest ()
{
}

PrivateOption::PrivateOption (const PrivateOption &p) :
    name (p.name),
    type (p.type),
    value (p.value),
    rest (p.rest)
{
}

