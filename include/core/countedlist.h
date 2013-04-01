/*
 * countedlist.h
 *
 *  Created on: Dec 15, 2009
 *      Author: erkin
 */

#ifndef COUNTEDLIST_H_
#define COUNTEDLIST_H_

#include <list>

template<typename value_type,
         typename allocator_type = std::allocator<value_type> >
class CountedList : protected std::list<value_type, allocator_type>
{
public:
    typedef std::list<value_type, allocator_type> Base;
    typedef typename Base::size_type              size_type;
    typedef typename Base::iterator               iterator;
    typedef typename Base::const_iterator         const_iterator;
    typedef typename Base::reverse_iterator       reverse_iterator;
    typedef typename Base::const_reverse_iterator const_reverse_iterator;
    typedef typename Base::reference              reference;
    typedef typename Base::const_reference        const_reference;

    CountedList () : Base (), mCount (0) { }

    // Overriden std::list methods

    bool empty () const { return Base::empty (); }
    size_type size () const { return mCount; }
    size_type max_size() const { return Base::max_size(); }

    iterator begin () { return Base::begin (); }
    const_iterator begin () const { return Base::begin (); }
    iterator end () { return Base::end (); }
    const_iterator end () const { return Base::end (); }

    reverse_iterator rbegin () { return Base::rbegin (); }
    const_reverse_iterator rbegin () const { return Base::rbegin (); }
    reverse_iterator rend () { return Base::rend (); }
    const_reverse_iterator rend () const { return Base::rend (); }

    reference front () { return Base::front (); }
    const_reference front () const { return Base::front (); }
    reference back () { return Base::back (); }
    const_reference back () const { return Base::back (); }

    void reverse () { Base::reverse (); }
    void sort () { Base::sort (); }

    template<typename _StrictWeakOrdering>
    void sort(_StrictWeakOrdering __comp) { Base::sort (__comp); }

    void resize (size_type __new_size, value_type __x = value_type ())
    {
	mCount = __new_size;
	Base::resize (__new_size, __x);
    }
    void clear ()
    {
	mCount = 0;
	Base::clear ();
    }
    void push_front (const value_type& __x)
    {
	mCount++;
	Base::push_front (__x);
    }
    void push_back (const value_type& __x)
    {
	mCount++;
	Base::push_back (__x);
    }
    void pop_front ()
    {
	if (mCount > 0)
	    mCount--;
	Base::pop_front ();
    }
    void pop_back ()
    {
	if (mCount > 0)
	    mCount--;
	Base::pop_back ();
    }
    iterator insert (iterator __position, const value_type& __x)
    {
	mCount++;
	return Base::insert (__position, __x);
    }
    void insert (iterator __position, size_type __n, const value_type& __x)
    {
	mCount += __n;
	Base::insert (__position, __n, __x);
    }
    iterator erase (iterator __position)
    {
	if (mCount > 0)
	    mCount--;
	return Base::erase (__position);
    }
    void remove (const value_type& __value)
    {
	if (mCount > 0)
	    mCount--;
	Base::remove (__value);
    }

protected:
    size_type mCount;
};

#endif /* COUNTEDLIST_H_ */
