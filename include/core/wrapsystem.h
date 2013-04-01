/*
 * Copyright © 2008 Dennis Kasprzyk
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
 */

#ifndef _COMPWRAPSYSTEM_H_
#define _COMPWRAPSYSTEM_H_

#include <stdlib.h>
#include <vector>
#include <algorithm>

#define WRAPABLE_DEF(func, ...)			 \
{						 \
    mHandler-> func ## SetEnabled (this, false); \
    return mHandler-> func (__VA_ARGS__);	 \
}

#define WRAPABLE_HND(num,itype,rtype, func, ...)	\
   rtype func (__VA_ARGS__);				\
   void  func ## SetEnabled (itype *obj, bool enabled)	\
   {							\
       functionSetEnabled (obj, num, enabled);		\
   }							\
   unsigned int func ## GetCurrentIndex ()		\
   {							\
       return mCurrFunction[num];			\
   }							\
   void func ## SetCurrentIndex (unsigned int index)	\
   {							\
       mCurrFunction[num] = index;			\
   }                                                    \
   enum { func ## Index = num };

// For compatability ignore num and forward
#define WRAPABLE_HND_FUNC(num, func, ...)				\
    WRAPABLE_HND_FUNCTN(func, __VA_ARGS__)

// New macro that doesn't need magic number
#define WRAPABLE_HND_FUNCTN(func, ...)				        \
{									\
    enum { num = func ## Index };                                       \
    unsigned int curr = mCurrFunction[num];				\
    while (mCurrFunction[num] < mInterface.size () &&			\
           !mInterface[mCurrFunction[num]].enabled[num])		\
        mCurrFunction[num]++;						\
    if (mCurrFunction[num] < mInterface.size ())			\
    {									\
	mInterface[mCurrFunction[num]++].obj-> func (__VA_ARGS__);	\
	mCurrFunction[num] = curr;					\
	return;								\
    }									\
    mCurrFunction[num] = curr;						\
}

// For compatability ignore num and forward
#define WRAPABLE_HND_FUNC_RETURN(num, rtype, func, ...)			\
    WRAPABLE_HND_FUNCTN_RETURN(rtype, func, __VA_ARGS__)

// New macro that doesn't need magic number
#define WRAPABLE_HND_FUNCTN_RETURN(rtype, func, ...)			\
{									\
    enum { num = func ## Index };                                       \
    unsigned int curr = mCurrFunction[num];				\
    while (mCurrFunction[num] < mInterface.size () &&			\
           !mInterface[mCurrFunction[num]].enabled[num])		\
        mCurrFunction[num]++;						\
    if (mCurrFunction[num] < mInterface.size ())			\
    {									\
	rtype rv = mInterface[mCurrFunction[num]++].obj-> func (__VA_ARGS__); \
	mCurrFunction[num] = curr;					\
	return rv;							\
    }									\
    mCurrFunction[num] = curr;						\
}

template <typename T, typename T2>
class WrapableInterface {
    protected:
	WrapableInterface () : mHandler (0) {}
	virtual ~WrapableInterface ()
	{
	    if (mHandler)
		mHandler->unregisterWrap (static_cast<T2*> (this));
	}

	void setHandler (T *handler, bool enabled = true)
	{
	    if (mHandler)
		mHandler->unregisterWrap (static_cast<T2*> (this));
	    if (handler)
		handler->registerWrap (static_cast<T2*> (this), enabled);
	    mHandler = handler;
	}
        T *mHandler;
};

template <typename T, unsigned int N>
class WrapableHandler : public T
{
    public:
	void registerWrap (T *, bool);
	void unregisterWrap (T *);

	unsigned int numWrapClients () { return mInterface.size (); }

    protected:

	struct Interface
	{
            Interface(T *obj, bool enable) : obj(obj)
            {
                std::fill_n(this->enabled, N, enable);
            }

            T    *obj;
            bool enabled[N];
	};

	WrapableHandler () : mInterface ()
	{
            std::fill_n(mCurrFunction, N, 0);
        }

	~WrapableHandler ()
	{
	    mInterface.clear ();
        }

	void functionSetEnabled (T *, unsigned int, bool);

        unsigned int mCurrFunction[N];
        std::vector<Interface> mInterface;
};

template <typename T, unsigned int N>
void WrapableHandler<T,N>::registerWrap (T *obj, bool enabled)
{
    mInterface.insert (mInterface.begin (), Interface(obj, enabled));
}

template <typename T, unsigned int N>
void WrapableHandler<T,N>::unregisterWrap (T *obj)
{
    typedef typename std::vector<Interface>::iterator iterator;
    for (iterator it = mInterface.begin (); it != mInterface.end (); ++it)
    {
	if (it->obj == obj)
	{
	    mInterface.erase (it);
	    break;
	}
    }
}

template <typename T, unsigned int N>
void WrapableHandler<T,N>::functionSetEnabled (T *obj, unsigned int num,
					       bool enabled)
{
    typedef typename std::vector<Interface>::iterator iterator;
    for (iterator it = mInterface.begin (); it != mInterface.end (); ++it)
    {
	if (it->obj == obj)
	{
	    it->enabled[num] = enabled;
	    break;
	}
    }
}

#endif
