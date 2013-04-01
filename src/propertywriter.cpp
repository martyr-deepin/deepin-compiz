/*
 * Copyright © 2010 Sam Spilsbury
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
 * Authors: Sam Spilsbury <smspillaz@gmail.com>
 */

#include <core/propertywriter.h>
#include <core/screen.h>

PropertyWriter::PropertyWriter ()
{
}

PropertyWriter::PropertyWriter (CompString propName,
				CompOption::Vector &readTemplate)
{
    mPropertyValues = readTemplate;
    mAtom = XInternAtom (screen->dpy (), propName.c_str (), 0);
}

void
PropertyWriter::setReadTemplate (const CompOption::Vector &readTemplate)
{
    mPropertyValues = readTemplate;
}

const CompOption::Vector &
PropertyWriter::getReadTemplate ()
{
    return mPropertyValues;
}

bool
PropertyWriter::updateProperty (Window		  	 id,
				CompOption::Vector &propertyData,
				int			 type)
{
    int count = 0;


    if (type != XA_STRING)
    {
        long int data[propertyData.size ()];

        foreach (CompOption &o, propertyData)
        {
	    switch (o.type ())
	    {
	        case CompOption::TypeBool:
		    data[count] = o.value ().b ();
		    break;
	        case CompOption::TypeInt:
		    data[count] = o.value ().i ();
		    break;
	        default:
		    data[count] = 0;
		    break;
	    }

	    count++;
        }

        XChangeProperty (screen->dpy (), id,
		         mAtom, type, 32,
		         PropModeReplace,  (unsigned char *)data,
		         propertyData.size ());
    }
    else
    {
        char * data[propertyData.size ()];
        XTextProperty prop;
        bool   ok = true;

        foreach (CompOption &o, propertyData)
        {
	    switch (o.type ())
	    {
	        case CompOption::TypeString:
		    data[count] = (char *) o.value ().s ().c_str ();
		    break;
	        default:
		    data[count] = NULL;
		    break;
	    }

	    count++;
        }

        for (int i = 0; i < count; i++)
        {
	    if (data[i] == NULL)
	    {
	        ok = false;
	    }
        }

        if (ok)
        {
	    if (XStringListToTextProperty (data, count, &prop))
	    {
	        XSetTextProperty (screen->dpy (), id, &prop, mAtom);
		XFree (prop.value);
	    }
        }
    }

    return true;
}

void
PropertyWriter::deleteProperty (Window id)
{
    XDeleteProperty (screen->dpy (), id, mAtom);
}

const CompOption::Vector &
PropertyWriter::readProperty (Window id)
{
    Atom 	  type;
    int  	  retval, fmt;
    unsigned long nitems, exbyte;
    long int	  *data;

    if (mPropertyValues.empty ())
	return mPropertyValues;

    retval = XGetWindowProperty (screen->dpy (), id, mAtom, 0,
    				 mPropertyValues.size (), False, XA_CARDINAL,
    				 &type, &fmt, &nitems, &exbyte,
    				 (unsigned char **)&data);

    if (retval == Success && !mPropertyValues.empty ())
    {
	int  count = 0;

	if (type == XA_CARDINAL && fmt == 32 &&
	    nitems == mPropertyValues.size ())
	{
	    foreach (CompOption &o, mPropertyValues)
	    {
		CompOption::Value tmpVal;
		switch (mPropertyValues.at (count).type ())
		{
		    case CompOption::TypeBool:
			tmpVal = CompOption::Value ((bool) data[count]);
			o.set (tmpVal);
			break;
		    case CompOption::TypeInt:
			tmpVal = CompOption::Value ((int) data[count]);
			o.set (tmpVal);
			break;
		    default:
			tmpVal = CompOption::Value (CompOption::Value (0));
			o.set (tmpVal);
			break;
		}

		count++;
	    }

	    XFree (data);

	    return mPropertyValues;
	}
	else if (type == XA_STRING && fmt == 8)
	{
	    XTextProperty tProp;
	    retval = XGetTextProperty (screen->dpy (), id, &tProp, mAtom);

	    if (tProp.value)
	    {
	        int  retCount = 0;
	        char **tData = NULL;

	        XTextPropertyToStringList (&tProp, &tData, &retCount);

	        if (retCount == (int) mPropertyValues.size ())
	        {
		    foreach (CompOption &o, mPropertyValues)
		    {
		        CompOption::Value tmpVal;
		        tmpVal = CompOption::Value (CompString ((char *) tData[count]));

		        o.set (tmpVal);

		        count++;
		    }

		    XFreeStringList (tData);
		    XFree (data);
		    XFree (tProp.value);

		    return mPropertyValues;
	        }
	        else
	        {
		    XFreeStringList (tData);
		    XFree (data);
		    XFree (tProp.value);

		    return nilValues;
	        }
	    }
	    else
	    {
		XFree (data);
		XFree (tProp.value);

	        return nilValues;
	    }
	}
	else if (fmt != 0)
	{
	    XFree (data);
	}
    }
    else
    {
	return mPropertyValues;
    }

    return mPropertyValues;
}
