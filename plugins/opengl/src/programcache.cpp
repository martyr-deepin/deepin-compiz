/*
 * Copyright © 2011 Linaro Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Linaro Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Linaro Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * LINARO LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL LINARO LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Travis Watkins <travis.watkins@linaro.org>
 */

#include <boost/shared_ptr.hpp>
#include <opengl/programcache.h>
#include "privates.h"

typedef std::list<std::string> access_history_t;
typedef std::pair<boost::shared_ptr<GLProgram>, access_history_t::iterator> value;

static GLProgram *
compileProgram (std::string name, std::list<const GLShaderData*> shaders)
{
    std::list<const GLShaderData*>::const_iterator it;
    std::string vertex_shader;
    std::string fragment_shader;
    std::string vertex_functions = "";
    std::string vertex_function_calls = "";
    std::string fragment_functions = "";
    std::string fragment_function_calls = "";
    int vpos, vcallpos, fpos, fcallpos;

    for (it = shaders.begin (); it != shaders.end (); ++it)
    {
	//find the special shaders to put the rest in
	if ((*it)->vertexShader.find ("@VERTEX_FUNCTIONS@") != std::string::npos)
	{
	    vertex_shader = (*it)->vertexShader;
	}
	else
	{
	    if ((*it)->vertexShader.length ())
	    {
		vertex_functions += (*it)->vertexShader;
		vertex_function_calls += (*it)->name + "_vertex();\n";
	    }
	}

	if ((*it)->fragmentShader.find ("@FRAGMENT_FUNCTIONS@") != std::string::npos)
	{
	    fragment_shader = (*it)->fragmentShader;
	}
	else
	{
	    if ((*it)->fragmentShader.length ())
	    {
		fragment_functions += (*it)->fragmentShader;
		fragment_function_calls += (*it)->name + "_fragment();\n";
	    }
	}
    }

    // put shader functions and function calls into the main shader
    vpos = vertex_shader.find ("@VERTEX_FUNCTIONS@");
    vertex_shader.replace (vpos, 18, vertex_functions);

    vcallpos = vertex_shader.find ("@VERTEX_FUNCTION_CALLS@");
    vertex_shader.replace (vcallpos, 23, vertex_function_calls);

    fpos = fragment_shader.find ("@FRAGMENT_FUNCTIONS@");
    fragment_shader.replace (fpos, 20, fragment_functions);

    fcallpos = fragment_shader.find ("@FRAGMENT_FUNCTION_CALLS@");
    fragment_shader.replace (fcallpos, 25, fragment_function_calls);

    return new GLProgram (vertex_shader, fragment_shader);
}

class PrivateProgramCache
{
    public:
	PrivateProgramCache (size_t);

	const size_t                 capacity;
	access_history_t             access_history;
	std::map<std::string, value> cache;

	void insert (std::string, GLProgram *);
	void evict ();
};

GLProgramCache::GLProgramCache (size_t capacity) :
    priv (new PrivateProgramCache (capacity))
{
    assert (priv->capacity != 0);
}

GLProgramCache::~GLProgramCache ()
{
    delete priv;
}
 
GLProgram* GLProgramCache::operator () (std::list<const GLShaderData*> shaders)
{
    std::list<const GLShaderData*>::const_iterator name_it;
    std::string name;

    for (name_it = shaders.begin(); name_it != shaders.end(); ++name_it)
    {
	if (name.length () == 0)
	    name += (*name_it)->name;
	else
	    name += ":" + (*name_it)->name;
    }

    std::map<std::string, value>::iterator it = priv->cache.find (name);
 
    if (it == priv->cache.end ())
    {
	GLProgram *program = compileProgram (name, shaders);
	priv->insert (name, program);
	return program;
    }
    else
    {
	priv->access_history.splice (priv->access_history.end (),
	                             priv->access_history,
	                             (*it).second.second);
	(*it).second.second = priv->access_history.rbegin ().base ();

	return (*it).second.first.get ();
    }
}

PrivateProgramCache::PrivateProgramCache (size_t c) :
    capacity (c)
{
}

void PrivateProgramCache::insert (std::string name, GLProgram *program)
{
    assert (cache.find (name) == cache.end ());

    if (cache.size () == capacity)
	evict ();

    // update most recently used GLProgram
    access_history_t::iterator it = access_history.insert (access_history.end (), name);

    cache.insert (std::make_pair (name, std::make_pair (program, it)));
}

void PrivateProgramCache::evict ()
{
    assert (!access_history.empty ());

    // find least recently used GLProgram
    std::map<std::string, value>::iterator it = cache.find (access_history.front ());
    assert (it != cache.end ());
 
    cache.erase (it);
    access_history.pop_front ();
}

