/*
 * Copyright © 2011 Collabora Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Collabora Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Collabora Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * COLLABORA LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL LINARO LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Frederic Plourde <frederic.plourde@collabora.co.uk>
 */

#ifndef _COMPIZ_WATERSHADERS_H
#define _COMPIZ_WATERSHADERS_H


// This vertex shader is used to set water vertices ...
static std::string set_water_vertices_vertex_shader = "                     \n\
#ifdef GL_ES                                                                \n\
precision mediump float;                                                    \n\
#endif                                                                      \n\
                                                                            \n\
attribute vec3 position;                                                    \n\
                                                                            \n\
void main ()                                                                \n\
{                                                                           \n\
    gl_PointSize = 3.0;                                                     \n\
    gl_Position = vec4(position, 1.0);                                      \n\
                                                                            \n\
}";

// This fragment shader is used to draw water primitives ...
// we're only setting the height here, so we only care about the .w coord.
static std::string set_water_vertices_fragment_shader = "                   \n\
#ifdef GL_ES                                                                \n\
precision mediump float;                                                    \n\
#endif                                                                      \n\
                                                                            \n\
uniform float color;                                                        \n\
                                                                            \n\
void main ()                                                                \n\
{                                                                           \n\
    gl_FragColor = vec4(0.0, 0.0, 0.0, color);                              \n\
}";




// This vertex shader is used to update water vertices...
static std::string update_water_vertices_vertex_shader = "                  \n\
#ifdef GL_ES                                                                \n\
precision mediump float;                                                    \n\
#endif                                                                      \n\
                                                                            \n\
attribute vec3 position;                                                    \n\
attribute vec2 texCoord0;                                                   \n\
                                                                            \n\
varying vec2 vTexCoord0;                                                    \n\
                                                                            \n\
void main ()                                                                \n\
{                                                                           \n\
    vTexCoord0 = texCoord0;                                                 \n\
    gl_Position = vec4(position, 1.0);                                      \n\
                                                                            \n\
}";

// This fragment shader is used to compute new normal + height of water vertices.
// here we're using two input textures, previous and current.
static std::string update_water_vertices_fragment_shader = "                \n\
#ifdef GL_ES                                                                \n\
precision mediump float;                                                    \n\
#endif                                                                      \n\
                                                                            \n\
uniform sampler2D prevTex;                                                  \n\
uniform sampler2D currTex;                                                  \n\
                                                                            \n\
varying vec2 vTexCoord0;                                                    \n\
                                                                            \n\
uniform float timeLapse;                                                    \n\
uniform float fade;                                                         \n\
                                                                            \n\
void main ()                                                                \n\
{                                                                           \n\
    vec2 t01, t21, t10, t12;                                                \n\
    vec4 c01, c21, c10, c12;                                                \n\
    vec4 curr, prev, v;                                                     \n\
    float accel;                                                            \n\
                                                                            \n\
    // fetch current and previous normals                                   \n\
    prev = texture%s (prevTex, vTexCoord0);                                 \n\
    curr = texture%s (currTex, vTexCoord0);                                 \n\
                                                                            \n\
    // sample offsets                                                       \n\
    t01 = vTexCoord0 + vec2 (- %f, 0.0);                                    \n\
    t21 = vTexCoord0 + vec2 (  %f, 0.0);                                    \n\
    t10 = vTexCoord0 + vec2 ( 0.0,- %f);                                    \n\
    t12 = vTexCoord0 + vec2 ( 0.0,  %f);                                    \n\
                                                                            \n\
    // fetch necessary samples                                              \n\
    c01 = texture%s (currTex, t01);                                         \n\
    c21 = texture%s (currTex, t21);                                         \n\
    c10 = texture%s (currTex, t10);                                         \n\
    c12 = texture%s (currTex, t12);                                         \n\
                                                                            \n\
    // x/y normals from height                                              \n\
    v = vec4 (0.0, 0.0, 0.75, 0.0);                                         \n\
    v.x = c01.w - c21.w;                                                    \n\
    v.y = c12.w - c10.w;                                                    \n\
                                                                            \n\
    // bumpiness                                                            \n\
    v = normalize (v);                                                      \n\
                                                                            \n\
    // add scale and bias                                                   \n\
    v = (v * 0.5) + 0.5;                                                    \n\
                                                                            \n\
    // done with computing the normal, continue with computing              \n\
    // the next height value                                                \n\
    accel = (curr.w * -4.0) + (c10.w + c12.w + c01.w + c21.w);              \n\
                                                                            \n\
    // store new height in alpha component                                  \n\
    v.w = (accel * timeLapse) + ((curr.w * 2.0) - prev.w);                  \n\
                                                                            \n\
    // fade out height                                                      \n\
    v.w *= fade;                                                            \n\
                                                                            \n\
    gl_FragColor = v;                                                       \n\
}";




// This vertex shader is used when painting our bump map FX over
// final composited screen FBO
static std::string paint_water_vertices_vertex_shader = "                   \n\
#ifdef GL_ES                                                                \n\
precision mediump float;                                                    \n\
#endif                                                                      \n\
                                                                            \n\
attribute vec3 position;                                                    \n\
attribute vec2 texCoord0;                                                   \n\
                                                                            \n\
varying vec2 vTexCoord0;                                                    \n\
                                                                            \n\
void main ()                                                                \n\
{                                                                           \n\
    vTexCoord0 = texCoord0;                                                 \n\
    gl_Position = vec4(position, 1.0);                                      \n\
                                                                            \n\
}";

// This fragment shader is used to produce our dot3 bump mapping output,
// blended over final composited glScreen FBO.
// here we're using two input textures :
//   1) The final composited FBO color attachment over which we're
//      applying our bump map FX (baseTex)
//   2) The updated bump map (waveTex)
static std::string paint_water_vertices_fragment_shader = "                 \n\
#ifdef GL_ES                                                                \n\
precision mediump float;                                                    \n\
#endif                                                                      \n\
                                                                            \n\
uniform sampler2D baseTex;                                                  \n\
uniform sampler2D waveTex;                                                  \n\
                                                                            \n\
varying vec2 vTexCoord0;                                                    \n\
                                                                            \n\
uniform vec3  lightVec;                                                     \n\
uniform float offsetScale;                                                  \n\
                                                                            \n\
void main ()                                                                \n\
{                                                                           \n\
    vec4 normal = texture2D (waveTex, vTexCoord0);                          \n\
    float height = normal.w;                                                \n\
    vec2 offset;                                                            \n\
                                                                            \n\
    normal = normalize ((normal * 2.0) - 1.0);                              \n\
                                                                            \n\
    offset.x = normal.x * height * offsetScale/%d.0;                        \n\
    offset.y = normal.y * height * offsetScale/%d.0;                        \n\
    vec4 baseColor  = texture2D (baseTex, vTexCoord0 + offset);             \n\
                                                                            \n\
    float diffFact = dot (-normal.xyz, lightVec.xyz);                       \n\
    gl_FragColor = vec4 (vec3 (baseColor) + diffFact, 1.0);                 \n\
}";

#endif // _COMPIZ_WATERSHADERS_H

