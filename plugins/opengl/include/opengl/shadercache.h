/*
 * Copyright © 2012 Linaro Ltd.
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
 * Authors: Alexandros Frantzis <alexandros.frantzis@linaro.org>
 */
#ifndef GL_SHADER_CACHE_H_
#define GL_SHADER_CACHE_H_

#include <string>

/**
 * How to use a variable in a shader.
 */
enum GLShaderVariableType
{
    /** The variable is not used */
    GLShaderVariableNone,
    /** The variable value is held in a uniform */
    GLShaderVariableUniform,
    /** The variable value is held in a varying (from a vertex attribute) */
    GLShaderVariableVarying,
};

/**
 * Parameters that define a vertex-fragment shader pair.
 */
struct GLShaderParameters
{
    /** Whether this shader supports opacity */
    bool opacity;
    /** Whether this shader supports brightness */
    bool brightness;
    /** Whether this shader supports saturation */
    bool saturation;
    /** Whether this shader supports color and how */
    GLShaderVariableType color;
    /** Whether this shader supports normals and how */
    GLShaderVariableType normal;
    /** The number of textures this shader uses */
    int numTextures;

    /** Gets a minimalistic string representation of the parameters */
    std::string id() const;
    /** Gets a unique hash value for this set of parameters */
    int hash() const;
};

/**
 * An object representing a named vertex-fragment shader pair.
 */
struct GLShaderData
{
    std::string name;
    std::string vertexShader;
    std::string fragmentShader;
};

class PrivateShaderCache;

/**
 * A cache of vertex-fragment shader pairs (GLShaderData).
 */
class GLShaderCache
{
public:
    GLShaderCache ();

    /**
     * Gets the GLShaderData associated with the specified parameters.
     *
     * @param params the parameters to get the GLShaderData for.
     *
     * @return the GLShaderData
     */
    const GLShaderData &getShaderData (const GLShaderParameters &params);

private:
    PrivateShaderCache *priv;
};

#endif
