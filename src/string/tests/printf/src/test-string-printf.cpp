/*
 * Copyright © 2011 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL, LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL CANONICAL, LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authored by: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include <core/string.h>

#include <gtest/gtest.h>

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <typeinfo>

namespace compiz
{
namespace string
{
namespace printf_test
{

class Value
{
public:
    virtual ~Value ();

    typedef boost::shared_ptr<Value> Ptr;

protected:

    Value ()
    {
    }
    ;

    void *v;
};

template<typename T>
class TValue: public Value
{
public:
    TValue (const T &);
    ~TValue ();

    const T & value ();
};

template<typename T>
TValue<T>::TValue (const T &av)
{
    v = new T(av);
}

template<typename T>
TValue<T>::~TValue ()
{
    delete (reinterpret_cast<T *>(v));
}

template<typename T>
const T &
TValue<T>::value ()
{
    return *(reinterpret_cast<T *>(v));
}

Value::~Value ()
{
}

CompString get_format (const CompString &fmt, Value::Ptr v)
{
    if (fmt == "%i" || fmt == "%d")
	return compPrintf(fmt.c_str(),
		(boost::shared_static_cast<TValue<int> >(v))->value());
    if (fmt == "%f")
	return compPrintf(fmt.c_str(),
		(boost::shared_static_cast<TValue<float> >(v))->value());
    if (fmt == "%s")
	return compPrintf(
		fmt.c_str(),
		(boost::shared_static_cast<TValue<std::string> >(v))->value().c_str());
    if (fmt == "%x")
	return compPrintf(fmt.c_str(),
		(boost::shared_static_cast<TValue<int> >(v))->value());

    return "not_reached";
}

}
}
}

TEST(CompizString,PrintfTest)
{
    CompString s1;
    CompString s2;
    std::map<CompString, compiz::string::printf_test::Value::Ptr> formatValues;
    std::map<CompString, CompString> formatStrings;

    s1 = "foo";

    const char *other_foo = "foo";
    s2 = compPrintf ("%s", other_foo);

    ASSERT_EQ(s1, s2);

    s1 = "3";
    s2 = compPrintf ("%i", 3, NULL);

    ASSERT_EQ(s1, s2);

    s1 = "3.012600";
    s2 = compPrintf ("%f", 3.0126, NULL);

    ASSERT_EQ(s1, s2);

    s1 = "0x4f567";
    s2 = compPrintf ("0x%x", 0x4f567, NULL);

    ASSERT_EQ(s1, s2);

    formatValues["%i"] = boost::shared_static_cast <compiz::string::printf_test::Value> (compiz::string::printf_test::Value::Ptr (new compiz::string::printf_test::TValue<int> (6)));
    formatStrings["%i"] = CompString ("6");
    formatValues["%f"] = boost::shared_static_cast <compiz::string::printf_test::Value> (compiz::string::printf_test::Value::Ptr (new compiz::string::printf_test::TValue<float> (6.532)));
    formatStrings["%f"] = CompString ("6.532000");
    formatValues["%x"] = boost::shared_static_cast <compiz::string::printf_test::Value> (compiz::string::printf_test::Value::Ptr (new compiz::string::printf_test::TValue<int> (0x34fe5aa)));
    formatStrings["%x"] = CompString ("34fe5aa");
    formatValues["%d"] = boost::shared_static_cast <compiz::string::printf_test::Value> (compiz::string::printf_test::Value::Ptr (new compiz::string::printf_test::TValue<int> (2)));
    formatStrings["%d"] = CompString ("2");

    for (std::map <CompString, CompString>::iterator it = formatStrings.begin ();
	    it != formatStrings.end (); it++)
    {
	CompString str = compiz::string::printf_test::get_format (it->first, formatValues[it->first]);
	ASSERT_EQ(str, it->second);
    }
}
