#include "core/wrapsystem.h"

#include <gtest/gtest.h>

//#define TEST_OLD_MACROS

namespace {

class TestImplementation;

class TestInterface : public WrapableInterface<TestImplementation, TestInterface> {
public:
    TestInterface();
    ~TestInterface();

    virtual void testMethodReturningVoid() /* const */ = 0;
    virtual int testMethodReturningInt(int i) /* const */ = 0;

    static int testMethodReturningVoidCalls;
    static int testMethodReturningIntCalls;

private:
    TestInterface(TestInterface const&);
    TestInterface& operator=(TestInterface const&);
};

// Needs a magic number for the count of "wrappable" functions
class TestImplementation : public WrapableHandler<TestInterface, 2> {
public:

    // 1. need for magic numbers
    // 2. why can't we just pass &TestInterface::testMethod (and deduce return etc.
    // 3. relies on __VA_ARGS__ when an extra set of parentheses would be enough
    WRAPABLE_HND (0, TestInterface, void, testMethodReturningVoid)

    WRAPABLE_HND (1, TestInterface, int, testMethodReturningInt, int)

    static int testMethodReturningVoidCalls;
    static int testMethodReturningIntCalls;
};

class TestWrapper : public TestInterface {
    TestImplementation& impl;
public:

    TestWrapper(TestImplementation& impl)
        : impl(impl)
    { setHandler(&impl, true); }  // The need to remember this is a PITA

    ~TestWrapper()
    { setHandler(&impl, false); }  // The need to remember this is a PITA

    virtual void testMethodReturningVoid();
    virtual int testMethodReturningInt(int i);

    static int testMethodReturningVoidCalls;
    static int testMethodReturningIntCalls;

    void disableTestMethodReturningVoid() {
        impl.testMethodReturningVoidSetEnabled (this, false);
    }
};
} // (abstract) namespace


int TestWrapper::testMethodReturningVoidCalls = 0;
int TestInterface::testMethodReturningVoidCalls = 0;
int TestImplementation::testMethodReturningVoidCalls = 0;

int TestWrapper::testMethodReturningIntCalls = 0;
int TestInterface::testMethodReturningIntCalls = 0;
int TestImplementation::testMethodReturningIntCalls = 0;

// A pain these need definition after TestImplementation definition
TestInterface::TestInterface() {}
TestInterface::~TestInterface() {}

void TestInterface::testMethodReturningVoid() /* const */ {
    WRAPABLE_DEF (testMethodReturningVoid);
    testMethodReturningVoidCalls++;
}

int TestInterface::testMethodReturningInt(int i) {
    WRAPABLE_DEF (testMethodReturningInt, i);
    testMethodReturningIntCalls++;
    return i;
}

void TestImplementation::testMethodReturningVoid() {
#ifdef TEST_OLD_MACROS
    WRAPABLE_HND_FUNC(0, testMethodReturningVoid) // Magic number needs to match class definition
#else
    WRAPABLE_HND_FUNCTN(testMethodReturningVoid)
#endif
    testMethodReturningVoidCalls++;
}

int TestImplementation::testMethodReturningInt(int i) {
#ifdef TEST_OLD_MACROS
    WRAPABLE_HND_FUNC_RETURN(1, int, testMethodReturningInt, i) // Magic number needs to match class definition
#else
    WRAPABLE_HND_FUNCTN_RETURN(int, testMethodReturningInt, i)
#endif
    testMethodReturningIntCalls++;
    return i;
}

void TestWrapper::testMethodReturningVoid() {
    impl.testMethodReturningVoid();
    testMethodReturningVoidCalls++;
}

int TestWrapper::testMethodReturningInt(int i) {
    testMethodReturningIntCalls++;
    return impl.testMethodReturningInt(i);
}


TEST(WrapSystem, an_interface_never_gets_functions_called)
{
    TestInterface::testMethodReturningIntCalls = 0;

    TestImplementation imp;

    imp.testMethodReturningInt(1);
    ASSERT_EQ(0, TestInterface::testMethodReturningIntCalls);

    {
        TestWrapper wrap(imp);

        imp.testMethodReturningInt(1);
        ASSERT_EQ(0, TestInterface::testMethodReturningIntCalls);
    }

    imp.testMethodReturningInt(1);
    ASSERT_EQ(0, TestInterface::testMethodReturningIntCalls);
}

TEST(WrapSystem, an_interface_never_gets_void_functions_called)
{
    TestInterface::testMethodReturningVoidCalls = 0;

    TestImplementation imp;

    imp.testMethodReturningVoid();
    ASSERT_EQ(0, TestInterface::testMethodReturningVoidCalls);

    {
        TestWrapper wrap(imp);

        imp.testMethodReturningVoid();
        ASSERT_EQ(0, TestInterface::testMethodReturningVoidCalls);
    }

    imp.testMethodReturningVoid();
    ASSERT_EQ(0, TestInterface::testMethodReturningVoidCalls);
}

TEST(WrapSystem, an_implementation_gets_functions_called)
{
    TestImplementation::testMethodReturningVoidCalls = 0;

    TestImplementation imp;
    {
        TestWrapper wrap(imp);

        imp.testMethodReturningVoid();

        ASSERT_EQ(1, TestImplementation::testMethodReturningVoidCalls);
    }

    imp.testMethodReturningVoid();

    ASSERT_EQ(2, TestImplementation::testMethodReturningVoidCalls);
}

TEST(WrapSystem, a_wrapper_gets_its_functions_called)
{
    TestWrapper::testMethodReturningVoidCalls = 0;

    TestImplementation imp;
    {
        TestWrapper wrap(imp);

        imp.testMethodReturningVoid();

        ASSERT_EQ(1, TestWrapper::testMethodReturningVoidCalls);
    }

    imp.testMethodReturningVoid();

    ASSERT_EQ(1, TestWrapper::testMethodReturningVoidCalls);
}

TEST(WrapSystem, a_wrapper_doesnt_get_disabled_functions_called)
{
    TestWrapper::testMethodReturningVoidCalls = 0;

    TestImplementation imp;
    {
        TestWrapper wrap(imp);

        wrap.disableTestMethodReturningVoid();

        imp.testMethodReturningVoid();

        ASSERT_EQ(0, TestWrapper::testMethodReturningVoidCalls);
    }
}

TEST(WrapSystem, two_wrappers_get_their_functions_called)
{
    TestWrapper::testMethodReturningVoidCalls = 0;

    TestImplementation imp;
    {
        TestWrapper wrap1(imp);
        TestWrapper wrap2(imp);

        imp.testMethodReturningVoid();

        ASSERT_EQ(2, TestWrapper::testMethodReturningVoidCalls);
    }

    imp.testMethodReturningVoid();

    ASSERT_EQ(2, TestWrapper::testMethodReturningVoidCalls);
}

