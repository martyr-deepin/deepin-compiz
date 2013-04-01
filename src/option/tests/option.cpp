#include <gtest/gtest.h>

#include "core/core.h"
#include "core/action.h"
#include "core/match.h"
#include "core/option.h"

namespace {

    template<typename T>
    void
    check_type_value(CompOption::Type type, const T & value)
    {
	CompOption::Value v;
	v.set(value);
	ASSERT_EQ(v.type(),type);
	ASSERT_EQ (v.get<T>(),value);
    }
    
    template<>
    void
    check_type_value(CompOption::Type type, const unsigned short (& value)[4])
    {
	CompOption::Value v;
	v.set(value);
	ASSERT_EQ(v.type(),type);
	unsigned short * color = v.get<unsigned short*>();
	ASSERT_NE((void*)0, color);
	for (int i = 0; i != 4; ++i) ASSERT_EQ(value[i], color[i]);
    }
    
    unsigned short testColor[4] = {255, 0, 255, 0};
    unsigned short testColor2[4] = {0, 255, 0, 255};

    template<typename T>
    void
    check_list_type(CompOption::Type listType, CompOption::Value::Vector &list)
    {
	CompOption::Value vl;
	vl.set (list);

	ASSERT_EQ (vl.type (), CompOption::TypeList);
	ASSERT_EQ (vl.get <CompOption::Value::Vector> (), list);

	for (CompOption::Value::Vector::const_iterator it = vl.get <CompOption::Value::Vector> ().begin ();
	     it != vl.get <CompOption::Value::Vector> ().end ();
	     it++)
	{
	    T inst;
	    CompOption::Value value (inst);

	    const CompOption::Value &v (*it);

	    ASSERT_EQ (v.type (), value.type ());
	}
    }
}

TEST(CompOption,Value)
{

    check_type_value<bool> (CompOption::TypeBool, true);
    check_type_value<bool> (CompOption::TypeBool, false);

    check_type_value<int> (CompOption::TypeInt, 1);
    check_type_value<float> (CompOption::TypeFloat, 1.f);
    check_type_value<CompString> (CompOption::TypeString, CompString ("Check"));
    check_type_value<CompString> (CompOption::TypeString, "core");
    
    check_type_value<CompAction> (CompOption::TypeAction, CompAction());
    check_type_value<CompMatch> (CompOption::TypeMatch, CompMatch());

    check_type_value<unsigned short[4]> (CompOption::TypeColor, testColor);
	
    check_type_value<CompOption::Value::Vector> (CompOption::TypeList, CompOption::Value::Vector(5));

    CompOption::Value v1, v2;
    ASSERT_EQ (v1,v2);
    v1.set (CompString("SomeString"));
    ASSERT_TRUE(v1 != v2);

    CompOption::Value::Vector vec;
    CompOption::Value v;

    v.set (true);
    vec.push_back (v);
    vec.push_back (v);

    check_list_type<bool> (CompOption::TypeBool, vec);

    vec.clear ();
    v.set (CompString ("foo"));
    vec.push_back (v);
    vec.push_back (v);

    check_list_type<CompString> (CompOption::TypeString, vec);
}

TEST(CompOption,Color)
{

    CompOption::Value value(testColor);

    unsigned short * color = value.c();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor[i], color[i]);

    color = value.get<unsigned short*>();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor[i], color[i]);

    value.set(testColor2);

    color = value.c();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor2[i], color[i]);

    color = value.get<unsigned short*>();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor2[i], color[i]);

    CompOption::Value v;

    v.set (testColor);

    color = v.c();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor[i], color[i]);

    color = v.get<unsigned short*>();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor[i], color[i]);

    v.set(testColor2);

    color = v.c();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor2[i], color[i]);

    color = v.get<unsigned short*>();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor2[i], color[i]);

    v.set (static_cast <short unsigned int *> (testColor));

    color = v.c();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor[i], color[i]);

    color = v.get<unsigned short*>();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor[i], color[i]);

    v.set(testColor2);

    color = v.c();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor2[i], color[i]);

    color = v.get<unsigned short*>();
    ASSERT_NE((void*)0, color);
    for (int i = 0; i != 4; ++i) ASSERT_EQ(testColor2[i], color[i]);
}

TEST(CompOption, Const)
{
    CompOption::Value non_const;
    CompOption::Value const& as_const(non_const);

    {
	CompString const expectOne("one");
	CompString const expectTwo("two");

	non_const = expectOne;
	ASSERT_EQ(expectOne, non_const.s());
	ASSERT_EQ(expectOne, as_const.s());

	non_const = expectTwo;
	ASSERT_EQ(expectTwo, non_const.s());
	ASSERT_EQ(expectTwo, as_const.s());
    }

    {
	bool const expectOne = true;
	bool const expectTwo = false;

	non_const = expectOne;
	ASSERT_EQ(expectOne, non_const.b());
	ASSERT_EQ(expectOne, as_const.b());

	non_const = expectTwo;
	EXPECT_FALSE (non_const.b());
	EXPECT_FALSE (as_const.b());
    }

    {
	float const expectOne = 0.0;
	float const expectTwo = 42.0;

	non_const = expectOne;
	ASSERT_EQ(expectOne, non_const.f());
	ASSERT_EQ(expectOne, as_const.f());

	non_const = expectTwo;
	ASSERT_EQ(expectTwo, non_const.f());
	ASSERT_EQ(expectTwo, as_const.f());
    }
}
