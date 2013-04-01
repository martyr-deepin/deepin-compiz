/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 *          David Reveman <davidr@novell.com>
 */

#ifndef _COMPOPTION_H
#define _COMPOPTION_H

#include <core/string.h>

#include <boost/variant.hpp>

#include <vector>

class PrivateOption;
class PrivateValue;
class PrivateRestriction;

class CompAction;
class CompMatch;
class CompScreenImpl;


/**
 * A configuration option with boolean, int, float, String, Color, Key, Button,
 * Edge, Bell, or List.
 */
class CompOption
{
	/**
	 * Option data types
	 */
    public:
	typedef enum {
	    TypeBool,
	    TypeInt,
	    TypeFloat,
	    TypeString,
	    TypeColor,
	    TypeAction,
	    TypeMatch,
	    TypeList,
	    TypeKey,
	    TypeButton,
	    TypeEdge,
	    TypeBell,
	    /* internal use only */
	    TypeUnset
	} Type;

	/**
	 * A value of an Option
	 */
	class Value {
	    public:

		typedef std::vector <unsigned short> ColorVector;

		typedef boost::variant<
		      bool,
		      int,
		      float,
		      CompString,
		      boost::recursive_wrapper<ColorVector>,
		      boost::recursive_wrapper<CompAction>,
		      boost::recursive_wrapper<CompMatch>,
		      boost::recursive_wrapper<std::vector<Value> >
		    > variant_type;

		typedef std::vector<Value> Vector;

	    public:
		Value () : mListType(TypeUnset)
		{
		}

		template<typename T>
		Value( const T & t );

		Value( unsigned short const (&color)[4] ) : mListType(TypeUnset),
		    mValue (ColorVector (color, color + 4))
		{
		}

		Value( const char *c ) : mListType (TypeUnset),
		    mValue (CompString (c))
		{
		}

		~Value();

		Type
		type () const
		{
		    return static_cast<Type>(mValue.which());
		}

		Type
		listType () const
		{
		    return mListType;
		}

		template<typename T>
		void set (const T & t);

		void set( unsigned short const (&color)[4] )
		{
		    mValue = ColorVector (color, color + 4);
		}

		void set (const char *c)
		{
		    mValue = CompString (c);
		}

		template<typename T>
		const T & get () const;

		void
		set (Type t, const Vector & v);

		bool
		b () const;

		int
		i () const;

		float
		f () const;

		unsigned short*
		c () const;

		const CompString &
		s () const;

		CompString &
		s ();

		const CompMatch &
		match () const;

		CompMatch &
		match ();

		const CompAction &
		action () const;

		CompAction &
		action ();

		const Vector &
		list () const;

		Vector &
		list ();

		bool
		operator== (const Value & rhs) const;

		bool
		operator!= (const Value & rhs) const;

	    private:
		Type mListType;
		variant_type mValue;
	};

	/**
	 * TODO
	 */
	class Restriction {
	    public:
		Restriction ();
		Restriction (const Restriction &);
		~Restriction ();

		int iMin ();
		int iMax ();
		float fMin ();
		float fMax ();
		float fPrecision ();

		void set (int, int);
		void set (float, float, float);

		bool inRange (int);
		bool inRange (float);

		Restriction & operator= (const Restriction &rest);
	    private:
		PrivateRestriction *priv;
	};

	typedef std::vector<CompOption> Vector;

	/**
	 * TODO
	 */
	class Class {
	    public:
		virtual ~Class() {}
		virtual Vector & getOptions () = 0;

		virtual CompOption * getOption (const CompString &name);

		virtual bool setOption (const CompString &name,
					Value            &value) = 0;
	};

    public:
	CompOption ();
	CompOption (const CompOption &);
	CompOption (CompString name, Type type);
	~CompOption ();

	void setName (CompString name, Type type);
	void setName (const char *name, Type type);

	void reset ();

	CompString name ();

	Type type ();
	Value & value ();
	Restriction & rest ();

	bool set (Value &val);
	bool isAction ();

	CompOption & operator= (const CompOption &option);

    public:
	static CompOption * findOption (Vector &options, CompString name,
					unsigned int *index = NULL);

	static bool
	getBoolOptionNamed (const Vector& options,
			    const CompString& name,
			    bool defaultValue = false);

	static int
	getIntOptionNamed (const Vector& options,
			   const CompString& name,
			   int defaultValue = 0);

	static float
	getFloatOptionNamed (const Vector& options,
			     const CompString& name,
			     const float& defaultValue = 0.0);

	static CompString
	getStringOptionNamed (const Vector& options,
			      const CompString& name,
			      const CompString& defaultValue = "");

	static unsigned short *
	getColorOptionNamed (const Vector& options,
			     const CompString& name,
			     unsigned short *defaultValue);

	static CompMatch
	getMatchOptionNamed (const Vector& options,
			     const CompString& name,
			     const CompMatch& defaultValue);

	static CompString typeToString (Type type);

	static bool stringToColor (CompString     color,
				   unsigned short *rgba);

	static CompString colorToString (unsigned short *rgba);


	static bool setOption (CompOption  &o, Value &value);

    private:
	PrivateOption *priv;
};

namespace compiz {
namespace detail {

template<typename Type>
inline
Type const& CompOption_Value_get(CompOption::Value::variant_type const& mValue)
{
    try
    {
	return boost::get<Type> (mValue);
    }
    catch (...)
    {
	static Type inst;
	return inst;
    }
}

template<>
inline
short unsigned int * const& CompOption_Value_get<short unsigned int *>(CompOption::Value::variant_type const& mValue)
{
    try
    {
	 static short unsigned int * some = 0;
         CompOption::Value::ColorVector const& tmp(boost::get<CompOption::Value::ColorVector>(mValue));

	 some = const_cast<unsigned short *> (&(tmp[0]));
	 return some;
    }
    catch (...)
    {
         static short unsigned int * none = 0;
         return none;
    }
}

template<typename Type>
inline
void CompOption_Value_set (CompOption::Value::variant_type & mValue, Type &t)
{
    mValue = t;
}

template <>
inline
void CompOption_Value_set<unsigned short *> (CompOption::Value::variant_type & mValue, unsigned short * &t)
{
    mValue = CompOption::Value::ColorVector (t, t + 4);
}

}
}

template<typename T>
inline
const T & CompOption::Value::get () const
{
     return compiz::detail::CompOption_Value_get<T>(mValue);
}

template<typename T>
inline
void CompOption::Value::set (const T & t)
{
    return compiz::detail::CompOption_Value_set<T>(mValue, const_cast <T &> (t));
}

template<typename T>
inline
CompOption::Value::Value (const T & t) :
    mListType (CompOption::TypeUnset)
{
    set (t);
}

CompOption::Vector & noOptions ();

#endif
