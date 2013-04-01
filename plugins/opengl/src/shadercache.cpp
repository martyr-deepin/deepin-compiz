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
#include <map>
#include <sstream>

#include <opengl/shadercache.h>

/** 
 * Compares two GLShaderParameters objects.
 */
class GLShaderParametersComparer
{
public:
    bool operator()(const GLShaderParameters &left, const GLShaderParameters &right) const
    {
        return left.hash() < right.hash();
    }

};

typedef std::map<GLShaderParameters, GLShaderData, GLShaderParametersComparer> ShaderMapType;

/** 
 * Private data for GLPrivate
 */
class PrivateShaderCache
{
public:
    PrivateShaderCache() {}

    ShaderMapType::const_iterator addShaderData(const GLShaderParameters &params);

    std::string createVertexShader (const GLShaderParameters &params);
    std::string createFragmentShader (const GLShaderParameters &params);

    ShaderMapType shaderMap;
};

/**********************
 * GLShaderParameters *
 **********************/

int
GLShaderParameters::hash() const
{
    return static_cast<int>(opacity) |
           (static_cast<int>(brightness) << 1) |
           (static_cast<int>(saturation) << 2) |
           (static_cast<int>(color) << 3) |
           (static_cast<int>(normal) << 5) |
           (static_cast<int>(numTextures) << 8);
}

std::string
GLShaderParameters::id() const
{
    std::stringstream ss;

    ss << (opacity ? "t" : "f");
    ss << (brightness ? "t" : "f");
    ss << (saturation ? "t" : "f");
    ss << (color == GLShaderVariableNone ? "n" :
           color == GLShaderVariableUniform ? "u" : "v");

    ss << (normal == GLShaderVariableNone ? "n" :
           normal == GLShaderVariableUniform ? "u" : "v");
    ss << numTextures;

    return ss.str();
}

/*****************
 * GLShaderCache *
 *****************/

GLShaderCache::GLShaderCache () :
    priv (new PrivateShaderCache ())
{
}

const GLShaderData &
GLShaderCache::getShaderData (const GLShaderParameters &params)
{
    ShaderMapType::const_iterator iter;

    // Try to find a cached shader pair that matches the parameters.
    // If we don't have it cached, create it.
    if ((iter = priv->shaderMap.find (params)) == priv->shaderMap.end ())
        iter = priv->addShaderData (params);

    return iter->second;
}

/**********************
 * PrivateShaderCache *
 **********************/

ShaderMapType::const_iterator
PrivateShaderCache::addShaderData (const GLShaderParameters &params)
{
    GLShaderData shaderData;

    shaderData.name = params.id ();
    shaderData.fragmentShader = createFragmentShader (params);
    shaderData.vertexShader = createVertexShader (params);

    std::pair<ShaderMapType::iterator, bool> ret =
        shaderMap.insert(std::pair<GLShaderParameters, GLShaderData>(params,shaderData));

    return ret.first;
}

/** 
 * Creates a minimal vertex shader that can handle the GLShaderParameters.
 * 
 * @param params the GLShaderParameters the created shader should handle.
 * 
 * @return the shader string
 */
std::string
PrivateShaderCache::createVertexShader (const GLShaderParameters &params)
{
    std::stringstream ss;

    ss << "#ifdef GL_ES\n" <<
          "precision mediump float;\n" <<
          "#endif\n";

    ss << "uniform mat4 modelview;\n" <<
          "uniform mat4 projection;\n";

    ss << "attribute vec3 position;\n" <<
          "attribute vec3 normal;\n" <<
          "attribute vec4 color;\n" <<
          "attribute vec2 texCoord0;\n" <<
          "attribute vec2 texCoord1;\n" <<
          "attribute vec2 texCoord2;\n" <<
          "attribute vec2 texCoord3;\n";

    ss << "@VERTEX_FUNCTIONS@\n";

    if (params.color == GLShaderVariableVarying)
        ss << "varying vec4 vColor;\n";

    for (int i = 0; i < params.numTextures; i++)
        ss << "varying vec2 vTexCoord" << i << ";\n";

    ss << "void main() {\n";

    for (int i = 0; i < params.numTextures; i++)
        ss << "vTexCoord" << i << " = texCoord" << i <<";\n";

    if (params.color == GLShaderVariableVarying)
        ss << "vColor = color;\n";

    ss << "gl_Position = projection * modelview * vec4(position, 1.0);\n";

    ss << "@VERTEX_FUNCTION_CALLS@\n}";

    return ss.str();
}

/** 
 * Creates a minimal fragment shader that can handle the GLShaderParameters.
 * 
 * @param params the GLShaderParameters the created shader should handle.
 * 
 * @return the shader string
 */
std::string
PrivateShaderCache::createFragmentShader (const GLShaderParameters &params)
{
    std::stringstream ss;
    ss << "#ifdef GL_ES\n" <<
          "precision mediump float;\n" <<
          "#endif\n";

    ss << "uniform vec3 paintAttrib;\n";

    for (int i = 0; i < params.numTextures; i++) {
        ss << "uniform sampler2D texture" << i << ";\n";
        ss << "varying vec2 vTexCoord" << i << ";\n";
    }

    if (params.color == GLShaderVariableUniform)
        ss << "uniform vec4 singleColor;\n";
    else if (params.color == GLShaderVariableVarying)
        ss << "varying vec4 vColor;\n";

    ss << "@FRAGMENT_FUNCTIONS@\n";

    ss << "void main() {\n vec4 color = ";

    if (params.color == GLShaderVariableUniform)
        ss << "singleColor *";
    else if (params.color == GLShaderVariableVarying)
        ss << "vColor *";

    for (int i = 0; i < params.numTextures; i++)
        ss << " texture2D(texture" << i << ", vTexCoord" << i << ") *";

    ss << " 1.0;\n";

    if (params.saturation) {
	ss << "vec3 desaturated = color.rgb * vec3 (0.30, 0.59, 0.11);\n" <<
	      "desaturated = vec3 (dot (desaturated, color.rgb));\n" <<
	      "color.rgb = color.rgb * vec3 (paintAttrib.z) + desaturated *\n" <<
	      "            vec3 (1.0 - paintAttrib.z);\n";
    }

    if (params.brightness) {
	ss << "color.rgb = color.rgb * paintAttrib.y" <<
	      (params.opacity ? " * paintAttrib.x;\n" : ";\n") <<
	      (params.opacity ? "color.a = color.a * paintAttrib.x;\n" : "");
    }
    else if (params.opacity) {
	ss << "color = color * paintAttrib.x;\n";
    }

    ss << "gl_FragColor = color;\n";
    ss << "@FRAGMENT_FUNCTION_CALLS@\n}";

    return ss.str();
}
