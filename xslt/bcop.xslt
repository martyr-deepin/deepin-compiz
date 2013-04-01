<!--
  Compiz option code generator

  Copyright : (C) 2009 by Dennis Kasprzyk
  E-mail    : onestone@compiz.org
 
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output  method="text"/>

<!-- external parameters that could be passed to this document -->

    <xsl:param name="file">header</xsl:param>

<!-- *** main block *** -->
    
    <xsl:template  match="/compiz">
        <xsl:if test="plugin[@useBcop = 'true']">
            <xsl:choose>
              <xsl:when test="$file = 'source'">
                  <xsl:call-template name="sfile"/>
              </xsl:when>
              <xsl:otherwise>
                  <xsl:call-template name="hfile"/>
              </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>

<!-- *** header file generation *** -->

    <xsl:template name="hfile">
        <xsl:call-template name="license"/>
        <xsl:text>#ifndef _</xsl:text>
        <xsl:value-of select="$PLUGIN"/>
        <xsl:text>_OPTIONS_H
#define _</xsl:text>
        <xsl:value-of select="$PLUGIN"/>
        <xsl:text>_OPTIONS_H

#include &lt;core/core.h&gt;

</xsl:text>
        <xsl:call-template name="printClass"/>
        <xsl:call-template name="printFunctions"/>
        <xsl:text>

#endif
</xsl:text>
    </xsl:template>

<!-- *** cpp source file generation *** -->

    <xsl:template name="sfile">
        <xsl:call-template name="license"/>
            <xsl:text>
#include &lt;core/core.h&gt;

#include "</xsl:text>
        <xsl:value-of select="$plugin"/>
        <xsl:text>_options.h"

</xsl:text>
        <xsl:call-template name="printConstructor"/>
        <xsl:call-template name="printDestructor"/>
        <xsl:call-template name="printBaseSetGet"/>
    </xsl:template>

<!-- print get/set functions -->

    <xsl:template name="printFunctions">
        <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option">
            <xsl:choose>
                <xsl:when test="@type='int'">
                    <xsl:text>inline int
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().i ();
}

</xsl:text>
                </xsl:when>
                <xsl:when test="@type='float'">
                    <xsl:text>inline float
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().f ();
}

</xsl:text>
                </xsl:when>
                <xsl:when test="@type='bool'">
                    <xsl:text>inline bool
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().b ();
}

</xsl:text>
                </xsl:when>
                <xsl:when test="@type='string'">
                    <xsl:text>inline CompString
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().s ();
}

</xsl:text>
                </xsl:when>
                <xsl:when test="@type='match'">
                    <xsl:text>inline CompMatch &amp;
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().match ();
}

</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='color'">
                    <xsl:text>inline unsigned short *
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                                        <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().c ();
}

inline unsigned short
</xsl:text>
                    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Red ()
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().c ()[0];
}

inline unsigned short
</xsl:text>
                    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Green ()
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().c ()[1];
}


inline unsigned short
</xsl:text>
                    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Blue ()
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().c ()[2];
}


inline unsigned short
</xsl:text>
                    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Alpha ()
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().c ()[3];
}


</xsl:text>
                </xsl:when>
                <xsl:when test="@type='action' or @type='key' or @type='button' or @type='edge' or @type='bell'">
                    <xsl:text>inline CompAction &amp;
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().action ();
}

inline void
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::optionSet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Initiate (CompAction::CallBack init)
{
    mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().action ().setInitiate (init);
}

inline void
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::optionSet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Terminate (CompAction::CallBack term)
{
    mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().action ().setTerminate (term);
}

</xsl:text>
                </xsl:when>
                <xsl:when test="@type='list'">
                    <xsl:text>inline CompOption::Value::Vector &amp;
</xsl:text>
		    <xsl:value-of select="$Plugin"/>
		    <xsl:text>Options::</xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>
{
    return mOptions[</xsl:text>
                    <xsl:call-template name="printOptionsEnumName"/>
                    <xsl:text>].value ().list ();
}

</xsl:text>
                    <xsl:if test="./type[text() = 'int']/../desc/value">
                        <xsl:text>inline unsigned int
</xsl:text>
			<xsl:value-of select="$Plugin"/>
			<xsl:text>Options::optionGet</xsl:text>
			<xsl:call-template name="printOptionName"/>
			<xsl:text>Mask ()
{
    return m</xsl:text>
                    <xsl:call-template name="printOptionName"/>
                    <xsl:text>Mask;
}

</xsl:text>
                    </xsl:if>
                </xsl:when>
            </xsl:choose>

            <xsl:text>inline void
</xsl:text>
	    <xsl:value-of select="$Plugin"/>
	    <xsl:text>Options::optionSet</xsl:text>
            <xsl:call-template name="printOptionName"/>
            <xsl:text>Notify (</xsl:text>
            <xsl:value-of select="$Plugin"/>
            <xsl:text>Options::ChangeNotify notify)
{
    mNotify[</xsl:text>
            <xsl:call-template name="printOptionsEnumName"/>
            <xsl:text>] = notify;
}

</xsl:text>
        </xsl:for-each>
    </xsl:template>

<!-- initialze option functions generation -->

    <xsl:template name="initBoolValue">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.set(</xsl:text>
        <xsl:choose>
            <xsl:when test="./text()">
		<xsl:call-template name="print">
		    <xsl:with-param name="text">
			<xsl:value-of select="./text()"/>
		    </xsl:with-param>
		</xsl:call-template>
		</xsl:when>
            <xsl:otherwise>
                <xsl:text>false</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>);
</xsl:text>
    </xsl:template>

    <xsl:template name="initIntValue">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.set((int) </xsl:text>
        <xsl:choose>
            <xsl:when test="./text()">
		<xsl:value-of select="./text()"/>
	    </xsl:when>
            <xsl:otherwise>
                <xsl:text>0</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>);
</xsl:text>
    </xsl:template>

    <xsl:template name="initFloatValue">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.set((float) </xsl:text>
        <xsl:choose>
            <xsl:when test="./text()">
		<xsl:value-of select="./text()"/>
	    </xsl:when>
            <xsl:otherwise>
                <xsl:text>0.0</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>);
</xsl:text>
    </xsl:template>

    <xsl:template name="initStringValue">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.set(CompString ("</xsl:text>
        <xsl:choose>
            <xsl:when test="./text()">
		<xsl:value-of select="./text()"/>
	    </xsl:when>
            <xsl:otherwise>
                <xsl:text></xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>"));
</xsl:text>
    </xsl:template>

    <xsl:template name="initMatchValue">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.set(CompMatch ("</xsl:text>
        <xsl:choose>
            <xsl:when test="./text()">
		<xsl:value-of select="./text()"/>
	    </xsl:when>
            <xsl:otherwise>
                <xsl:text></xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>"));
</xsl:text>
        <xsl:value-of select="$value"/>
        <xsl:text>.match ().update ();
</xsl:text>
    </xsl:template>

    <xsl:template name="initColorValue">
        <xsl:param name="value"/>
        <xsl:text>    color[0] = </xsl:text>
	<xsl:choose>
            <xsl:when test="./red/text()">
                <xsl:text>MAX (0, MIN (0xffff, </xsl:text>
                <xsl:value-of select="./red/text()"/>
                <xsl:text>));
</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>0;
</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    color[1] = </xsl:text>
	<xsl:choose>
            <xsl:when test="./green/text()">
                <xsl:text>MAX (0, MIN (0xffff, </xsl:text>
                <xsl:value-of select="./green/text()"/>
                <xsl:text>));
</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>0;
</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    color[2] = </xsl:text>
	<xsl:choose>
            <xsl:when test="./blue/text()">
                <xsl:text>MAX (0, MIN (0xffff, </xsl:text>
                <xsl:value-of select="./blue/text()"/>
                <xsl:text>));
</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>0;
</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>    color[3] = </xsl:text>
	<xsl:choose>
            <xsl:when test="./alpha/text()">
                <xsl:text>MAX (0, MIN (0xffff, </xsl:text>
                <xsl:value-of select="./alpha/text()"/>
                <xsl:text>));
</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>0xffff;
</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
	<xsl:value-of select="$value"/>
        <xsl:text>.set(color);
</xsl:text>
    </xsl:template>

    <xsl:template name="initActionValue">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.set(CompAction ());
</xsl:text>
	<xsl:value-of select="$value"/>
        <xsl:text>.action ().setState (state);
</xsl:text>
    </xsl:template>

    <xsl:template name="initKeyValue">
        <xsl:param name="value"/>
        <xsl:text>    action = CompAction ();
</xsl:text>
        <xsl:text>    action.setState (state | CompAction::StateInitKey);
</xsl:text>
        <xsl:if test="default/text() and default/text() != 'disabled'">
            <xsl:text>    action.keyFromString ("</xsl:text>
            <xsl:value-of select="default/text()"/>
            <xsl:text>");
</xsl:text>
        </xsl:if>

        <xsl:value-of select="$value"/>
        <xsl:text>.set (action);
</xsl:text>
    </xsl:template>

    <xsl:template name="initButtonValue">
        <xsl:param name="value"/>
        <xsl:text>    action = CompAction ();
</xsl:text>
        <xsl:text>    action.setState (state | CompAction::StateInitButton);
</xsl:text>
        <xsl:if test="default/text() and default/text() != 'disabled'">
            <xsl:text>    action.buttonFromString ("</xsl:text>
            <xsl:value-of select="default/text()"/>
            <xsl:text>");
</xsl:text>
        </xsl:if>

        <xsl:value-of select="$value"/>
        <xsl:text>.set (action);
</xsl:text>
    </xsl:template>

    <xsl:template name="initEdgeValue">
        <xsl:param name="value"/>
        <xsl:text>    action = CompAction ();
</xsl:text>
        <xsl:text>    action.setState (state | CompAction::StateInitEdge);
</xsl:text>
        <xsl:text>    edge = 0;
</xsl:text>

        <xsl:for-each select="default/edge">
            <xsl:choose>
                <xsl:when test="@name = 'Left'">
                    <xsl:text>    edge |= 1;
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'Right'">
                    <xsl:text>    edge |= (1 &lt;&lt; 1);
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'Top'">
                    <xsl:text>    edge |= (1 &lt;&lt; 2);
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'Bottom'">
                    <xsl:text>    edge |= (1 &lt;&lt; 3);
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'TopLeft'">
                    <xsl:text>    edge |= (1 &lt;&lt; 4);
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'TopRight'">
                    <xsl:text>    edge |= (1 &lt;&lt; 5);
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'BottomLeft'">
                    <xsl:text>    edge |= (1 &lt;&lt; 6);
</xsl:text>
                </xsl:when>
                <xsl:when test="@name = 'BottomRight'">
                    <xsl:text>    edge |= (1 &lt;&lt; 7);
</xsl:text>
                </xsl:when>
            </xsl:choose>
        </xsl:for-each>

        <xsl:text>    action.setEdgeMask (edge);
</xsl:text>

        <xsl:value-of select="$value"/>
        <xsl:text>.set (action);
</xsl:text>
    </xsl:template>

    <xsl:template name="initBellValue">
        <xsl:param name="value"/>
        <xsl:text>    action = CompAction ();
</xsl:text>
        <xsl:text>    action.setState (state | CompAction::StateInitBell);
</xsl:text>
        <xsl:if test="default/text() and default/text() = 'true'">
            <xsl:text>    action.setBell (true);
</xsl:text>
        </xsl:if>

        <xsl:value-of select="$value"/>
        <xsl:text>.set (action);
</xsl:text>
    </xsl:template>

     <xsl:template name="initListValue">
        <xsl:param name="value"/>
        <xsl:text>    list.clear ();
</xsl:text>
        <xsl:for-each select="./value">
            <xsl:choose>
                <xsl:when test="../../type/text() = 'bool'">
                    <xsl:call-template name="initBoolValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'float'">
                    <xsl:call-template name="initFloatValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'int'">
                    <xsl:call-template name="initIntValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'string'">
                    <xsl:call-template name="initStringValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'match'">
                    <xsl:call-template name="initMatchValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'action'">
                    <xsl:call-template name="initActionValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'key'">
                    <xsl:call-template name="initKeyValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'button'">
                    <xsl:call-template name="initButtonValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'edge'">
                    <xsl:call-template name="initEdgeValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="../../type/text() = 'bell'">
                    <xsl:call-template name="initBellValue">
                        <xsl:with-param name="value">
                            <xsl:text>    value</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
            </xsl:choose>
            <xsl:text>    list.push_back (value);
</xsl:text>
        </xsl:for-each>

        <xsl:value-of select="$value"/>
        <xsl:text>.set (</xsl:text>
        <xsl:choose>
            <xsl:when test="../type/text() = 'bool'">
                <xsl:text>CompOption::TypeBool</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'int'">
                <xsl:text>CompOption::TypeInt</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'float'">
                <xsl:text>CompOption::TypeFloat</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'string'">
                <xsl:text>CompOption::TypeString</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'color'">
                <xsl:text>CompOption::TypeColor</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'action'">
                <xsl:text>CompOption::TypeAction</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'key'">
                <xsl:text>CompOption::TypeKey</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'button'">
                <xsl:text>CompOption::TypeButton</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'edge'">
                <xsl:text>CompOption::TypeEdge</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'bell'">
                <xsl:text>CompOption::TypeBell</xsl:text>
            </xsl:when>
            <xsl:when test="../type/text() = 'match'">
                <xsl:text>CompOption::TypeMatch</xsl:text>
            </xsl:when>
        </xsl:choose>
        <xsl:text>, list);
</xsl:text>

    </xsl:template>

    <xsl:template name="initActionState">
        <xsl:choose>
            <xsl:when test="./passive_grab/text() = 'false'">
                <xsl:text>    state = 0;
</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>    state = CompAction::StateAutoGrab;
</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="@type = 'edge' and ./nodelay/text() = 'true'">
            <xsl:text>    state |= CompAction::StateNoEdgeDelay;
</xsl:text>
        </xsl:if>
        <xsl:if test="./allowed[@key = 'true']">
            <xsl:text>    state |= CompAction::StateInitKey;
</xsl:text>
        </xsl:if>
        <xsl:if test="./allowed[@button = 'true']">
            <xsl:text>    state |= CompAction::StateInitButton;
</xsl:text>
        </xsl:if>
        <xsl:if test="./allowed[@bell = 'true']">
            <xsl:text>    state |= CompAction::StateInitBell;
</xsl:text>
        </xsl:if>
        <xsl:if test="./allowed[@edge = 'true']">
            <xsl:text>    state |= CompAction::StateInitEdge;
</xsl:text>
        </xsl:if>
        <xsl:if test="./allowed[@edgednd = 'true']">
            <xsl:text>    state |= CompAction::StateInitEdgeDnd;
</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template name="initIntRestriction">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.rest ().set (</xsl:text>
        <xsl:choose>
            <xsl:when test="./min/text()">
                <xsl:value-of select="./min/text()"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>MINSHORT</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>, </xsl:text>
        <xsl:choose>
            <xsl:when test="./max/text()">
                <xsl:value-of select="./max/text()"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>MAXSHORT</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>);
</xsl:text>
    </xsl:template>

    <xsl:template name="initFloatRestriction">
        <xsl:param name="value"/>
	<xsl:value-of select="$value"/>
        <xsl:text>.rest ().set (</xsl:text>
        <xsl:choose>
            <xsl:when test="./min/text()">
                <xsl:value-of select="./min/text()"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>MINSHORT</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>, </xsl:text>
        <xsl:choose>
            <xsl:when test="./max/text()">
                <xsl:value-of select="./max/text()"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>MAXSHORT</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>, </xsl:text>
        <xsl:choose>
            <xsl:when test="./precision/text()">
                <xsl:value-of select="./precision/text()"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>0.1f</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>);
</xsl:text>
    </xsl:template>


<!-- set option function generation -->

    <xsl:template name="setOptions">
        <xsl:text>    CompOption   *o;
    unsigned int index;

    o = CompOption::findOption (mOptions, name, &amp;index);

    if (!o)
        return false;

    switch (index)
    {
</xsl:text>
        <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option">
            <xsl:call-template name="setOption"/>
        </xsl:for-each>
        <xsl:text>        default:
            break;
    }
    return false;
</xsl:text>
    </xsl:template>

    <xsl:template name="setOption">
        <xsl:text>        case </xsl:text>
        <xsl:call-template name="printOptionsEnumName"/>
        <xsl:text>:
            if (o->set (value))
            {
</xsl:text>
        <xsl:if test="@type = 'list' and ./desc/value and ./type/text() = 'int'">
            <xsl:text>                m</xsl:text>
            <xsl:call-template name="printOptionName"/>
            <xsl:text>Mask = 0;
                foreach (CompOption::Value &amp;val, o->value ().list ())
                    m</xsl:text>
            <xsl:call-template name="printOptionName"/>
        <xsl:text>Mask |= (1 &lt;&lt; val.i ());
</xsl:text>
        </xsl:if>
        <xsl:text>                if (!mNotify[</xsl:text>
        <xsl:call-template name="printOptionsEnumName"/>
        <xsl:text>].empty ())
                    mNotify[</xsl:text>
        <xsl:call-template name="printOptionsEnumName"/>
        <xsl:text>] (o, </xsl:text>
        <xsl:call-template name="printOptionsEnumName"/>
        <xsl:text>);
                return true;
            }
            break;
</xsl:text>
    </xsl:template>



<!-- enum for options -->

    <xsl:template name="printOptionsEnum">
<xsl:text>enum Options {
</xsl:text>
        <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option">
            <xsl:text>            </xsl:text>
            <xsl:call-template name="printOptionsEnumNamePure"/>
            <xsl:text>,
</xsl:text>
        </xsl:for-each>
        <xsl:text>            OptionNum
        };

</xsl:text>
    </xsl:template>


<!-- generate enums/masks for restricted string options -->

    <xsl:template name="printOptionEnums">
                <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option[(@type = 'int' or (@type = 'list' and ./type/text() = 'int')) and ./desc/value]">
            <xsl:text>        enum </xsl:text>
            <xsl:call-template name="printOptionName"/>
            <xsl:text> {
</xsl:text>
            <xsl:for-each select="desc/*[(name() = 'name' and not(@xml:lang)) or name() = '_name']">
                <xsl:text>            </xsl:text>
                <xsl:call-template name="PrintCamel">
		    <xsl:with-param name="text">
			<xsl:value-of select="../../@name"/>
		    </xsl:with-param>
		</xsl:call-template>
                <xsl:call-template name="PrintCamel">
		    <xsl:with-param name="text">
			<xsl:value-of select="text()"/>
		    </xsl:with-param>
	        </xsl:call-template>
	        <xsl:text> = </xsl:text>
	        <xsl:value-of select="../value/text()"/>
	        <xsl:text>,
</xsl:text>
            </xsl:for-each>
            <xsl:text>        };

</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="printOptionMasks">
        <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list' and ./desc/value and ./type/text() = 'int']">
            <xsl:for-each select="desc/*[(name() = 'name' and not(@xml:lang)) or name() = '_name']">
                <xsl:text>        #define </xsl:text>
                <xsl:call-template name="PrintCamel">
		    <xsl:with-param name="text">
			<xsl:value-of select="../../@name"/>
		    </xsl:with-param>
		</xsl:call-template>
                <xsl:call-template name="PrintCamel">
		    <xsl:with-param name="text">
			<xsl:value-of select="text()"/>
		    </xsl:with-param>
	        </xsl:call-template>
	        <xsl:text>Mask (1 &lt;&lt; </xsl:text>
	        <xsl:value-of select="../value/text()"/>
	        <xsl:text>)
</xsl:text>
            </xsl:for-each>
            <xsl:text>

</xsl:text>
        </xsl:for-each>
    </xsl:template>

<!-- generate get/set function definitions -->

    <xsl:template name="printClass">
        <xsl:text>class </xsl:text>
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options : public CompOption::Class {
    public:
        </xsl:text>
        <xsl:call-template name="printOptionsEnum"/>
        <xsl:call-template name="printOptionEnums"/>
        <xsl:call-template name="printOptionMasks"/>
        <xsl:text>        typedef boost::function &lt;void (CompOption *opt, Options num)&gt; ChangeNotify;

        </xsl:text>
	<xsl:value-of select="$Plugin"/>
        <xsl:text>Options (bool init = true);
        virtual ~</xsl:text>
	<xsl:value-of select="$Plugin"/>
        <xsl:text>Options ();

	void initOptions ();

        virtual CompOption::Vector &amp; getOptions ();
        virtual bool setOption (const CompString &amp;name, CompOption::Value &amp;value);

</xsl:text>
        <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option">
            <xsl:choose>
                <xsl:when test="@type='int'">
                    <xsl:text>        int </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
</xsl:text>
                </xsl:when>
                <xsl:when test="@type='float'">
                    <xsl:text>        float </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='bool'">
                    <xsl:text>        bool </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='string'">
                    <xsl:text>        CompString </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='match'">
                    <xsl:text>        CompMatch &amp; </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='color'">
                    <xsl:text>        unsigned short * </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
        unsigned short optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Red ();
        unsigned short optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Green ();
        unsigned short optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Blue ();
        unsigned short optionGet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Alpha ();
</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='action' or @type='key' or @type='button' or @type='edge' or @type='bell'">
                    <xsl:text>        CompAction &amp; </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
        void </xsl:text>
		    <xsl:text>optionSet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Initiate (CompAction::CallBack init);
        void </xsl:text>
		    <xsl:text>optionSet</xsl:text>
		    <xsl:call-template name="printOptionName"/>
		    <xsl:text>Terminate (CompAction::CallBack term);
</xsl:text>
                </xsl:when>
                 <xsl:when test="@type='list'">
                    <xsl:text>        CompOption::Value::Vector &amp; </xsl:text>
                    <xsl:call-template name="printGetFunctionDef"/>
                    <xsl:text>;
</xsl:text>
                    <xsl:if test="./type[text() = 'int']/../desc/value">
                        <xsl:text>        unsigned int optionGet</xsl:text>
			<xsl:call-template name="printOptionName"/>
			<xsl:text>Mask ();
</xsl:text>
                    </xsl:if>
                </xsl:when>
            </xsl:choose>
	    <xsl:text>        void optionSet</xsl:text>
            <xsl:call-template name="printOptionName"/>
            <xsl:text>Notify (ChangeNotify notify);

</xsl:text>
        </xsl:for-each>
        <xsl:text>
    protected:
        CompOption::Vector mOptions;

    private:
        std::vector&lt;ChangeNotify&gt; mNotify;
</xsl:text>
        <xsl:for-each select="plugin[@name=$pName]/descendant-or-self::option[@type = 'list' and ./desc/value and ./type/text() = 'int']">
        <xsl:text>        unsigned int m</xsl:text>
        <xsl:call-template name="printOptionName"/>
        <xsl:text>Mask;
</xsl:text>
        </xsl:for-each>
        <xsl:text>
};


</xsl:text>
    </xsl:template>

    <xsl:template name="printGetFunctionDef">
	<xsl:text>optionGet</xsl:text>
        <xsl:call-template name="printOptionName"/>
        <xsl:text> ()</xsl:text>
    </xsl:template>

    <!-- compiz set/getOption(s) function generation -->

    <xsl:template name="printBaseSetGet">
        <xsl:text>
CompOption::Vector &amp;
</xsl:text>
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::getOptions ()
{
    return mOptions;
}

bool
</xsl:text>
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::setOption (const CompString &amp;name, CompOption::Value &amp;value)
{
</xsl:text>
        <xsl:call-template name="setOptions"/>
        <xsl:text>
}

</xsl:text>
    </xsl:template>

    <!-- constructor/destructor function generation -->

    <xsl:template name="printConstructor">
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::</xsl:text>
	<xsl:value-of select="$Plugin"/>
        <xsl:text>Options (bool init /* = true */) :
    mOptions (</xsl:text>
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::OptionNum),
    mNotify (</xsl:text>
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::OptionNum)
{
    if (init)
        initOptions ();
}

void
</xsl:text>
<xsl:value-of select="$Plugin"/><xsl:text>Options::initOptions ()
{
</xsl:text>
    <xsl:if test="/compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'action'] or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'key'] or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'button'] or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'edge'] or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'bell'] or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']/type/text() = 'action' or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']/type/text() = 'key' or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']/type/text() = 'button' or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']/type/text() = 'edge' or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']/type/text() = 'bell'">
        <xsl:text>    unsigned int state;
    CompAction action;
</xsl:text>
    </xsl:if>
    <xsl:if test="/compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'edge'] or
                  /compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']/type/text() = 'edge'">
        <xsl:text>    unsigned int edge;
</xsl:text>
    </xsl:if>
<xsl:if test="/compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'color']">
        <xsl:text>    unsigned short color[4];
</xsl:text>
    </xsl:if>
    <xsl:if test="/compiz/plugin[@name=$pName]/descendant-or-self::option[@type = 'list']">
        <xsl:text>    CompOption::Value::Vector list;
    CompOption::Value value;
</xsl:text>
    </xsl:if>
    <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option">
        <xsl:text>
    // </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>
</xsl:text>
        <xsl:variable name="opt">
            <xsl:text>    mOptions[</xsl:text>
	    <xsl:call-template name="printOptionsEnumName"/>
	    <xsl:text>]</xsl:text>
        </xsl:variable>
        <xsl:value-of select="$opt"/>
        <xsl:text>.setName ("</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>", </xsl:text>
        <xsl:choose>
	    <xsl:when test="@type = 'bool'">
		<xsl:text>CompOption::TypeBool</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'int'">
		<xsl:text>CompOption::TypeInt</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'float'">
		<xsl:text>CompOption::TypeFloat</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'string'">
		<xsl:text>CompOption::TypeString</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'color'">
		<xsl:text>CompOption::TypeColor</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'action'">
		<xsl:text>CompOption::TypeAction</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'key'">
		<xsl:text>CompOption::TypeKey</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'button'">
		<xsl:text>CompOption::TypeButton</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'edge'">
		<xsl:text>CompOption::TypeEdge</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'bell'">
		<xsl:text>CompOption::TypeBell</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'match'">
		<xsl:text>CompOption::TypeMatch</xsl:text>
	    </xsl:when>
	    <xsl:when test="@type = 'list'">
		<xsl:text>CompOption::TypeList</xsl:text>
	    </xsl:when>
	</xsl:choose>
	<xsl:text>);
</xsl:text>
            <xsl:choose>
                <xsl:when test="@type='bool'">
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initBoolValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
                <xsl:when test="@type='float'">
                    <xsl:call-template name="initFloatRestriction">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
			</xsl:with-param>
                    </xsl:call-template>
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initFloatValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
                <xsl:when test="@type='int'">
                    <xsl:call-template name="initIntRestriction">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
			</xsl:with-param>
                    </xsl:call-template>
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initIntValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
                <xsl:when test="@type='string'">
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initStringValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
                <xsl:when test="@type='match'">
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initMatchValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
                <xsl:when test="@type='color'">
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initColorValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
                <xsl:when test="@type='action'">
                    <xsl:call-template name="initActionState"/>
                    <xsl:call-template name="initActionValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="@type='key'">
		    <xsl:call-template name="initActionState"/>
                    <xsl:call-template name="initKeyValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    <xsl:if test="not (./passive_grab/text() = 'false')">
                        <xsl:text>    if (screen) screen->addAction (&amp;</xsl:text>
                        <xsl:text>mOptions[</xsl:text>
	                <xsl:call-template name="printOptionsEnumName"/>
	                <xsl:text>].value ().action ());
</xsl:text>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="@type='button'">
                    <xsl:call-template name="initActionState"/>
                    <xsl:call-template name="initButtonValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    <xsl:if test="not (./passive_grab/text() = 'false')">
                        <xsl:text>    if (screen) screen->addAction (&amp;</xsl:text>
                        <xsl:text>mOptions[</xsl:text>
	                <xsl:call-template name="printOptionsEnumName"/>
                        <xsl:text>].value ().action ());
</xsl:text>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="@type='edge'">
                    <xsl:call-template name="initActionState"/>
                    <xsl:call-template name="initEdgeValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    <xsl:if test="not (./passive_grab/text() = 'false')">
                        <xsl:text>    if (screen) screen->addAction (&amp;</xsl:text>
                        <xsl:text>mOptions[</xsl:text>
	                <xsl:call-template name="printOptionsEnumName"/>
                        <xsl:text>].value ().action ());
</xsl:text>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="@type='bell'">
                    <xsl:call-template name="initActionState"/>
                    <xsl:call-template name="initBellValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    <xsl:if test="not (./passive_grab/text() = 'false')">
                        <xsl:text>    if (screen) screen->addAction (&amp;</xsl:text>
                        <xsl:text>mOptions[</xsl:text>
	                <xsl:call-template name="printOptionsEnumName"/>
                        <xsl:text>].value ().action ());
</xsl:text>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="@type='list'">
                    <xsl:if test="not (./default[1])">
                        <xsl:text>    value.set (</xsl:text>
			<xsl:choose>
			    <xsl:when test="./type/text() = 'bool'">
				<xsl:text>CompOption::TypeBool</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'int'">
				<xsl:text>CompOption::TypeInt</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'float'">
				<xsl:text>CompOption::TypeFloat</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'string'">
				<xsl:text>CompOption::TypeString</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'color'">
				<xsl:text>CompOption::TypeColor</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'action'">
				<xsl:text>CompOption::TypeAction</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'key'">
				<xsl:text>CompOption::TypeKey</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'button'">
				<xsl:text>CompOption::TypeButton</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'edge'">
				<xsl:text>CompOption::TypeEdge</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'bell'">
				<xsl:text>CompOption::TypeBell</xsl:text>
			    </xsl:when>
			    <xsl:when test="./type/text() = 'match'">
				<xsl:text>CompOption::TypeMatch</xsl:text>
			    </xsl:when>
			</xsl:choose>
			<xsl:text>, CompOption::Value::Vector (0));
</xsl:text>
		        <xsl:value-of select="$opt"/>
			<xsl:text>.set (value);
</xsl:text>
		    </xsl:if>
                    <xsl:choose>
                        <xsl:when test="./type/text () = 'int'">
                            <xsl:call-template name="initIntRestriction">
                                <xsl:with-param name="value">
                                    <xsl:value-of select="$opt"/>
			         </xsl:with-param>
			    </xsl:call-template>
                        </xsl:when>
                        <xsl:when test="./type/text () = 'float'">
                            <xsl:call-template name="initFloatRestriction">
                                <xsl:with-param name="value">
                                    <xsl:value-of select="$opt"/>
			         </xsl:with-param>
			    </xsl:call-template>
                        </xsl:when>
                        <xsl:when test="./type/text () = 'action' or ./type/text () = 'key' or ./type/text () = 'button' or ./type/text () = 'edge' or ./type/text () = 'bell'">
                            <xsl:call-template name="initActionState"/>
                        </xsl:when>
                    </xsl:choose>
                    <xsl:for-each select="./default[1]">
                    <xsl:call-template name="initListValue">
                        <xsl:with-param name="value">
                            <xsl:value-of select="$opt"/>
                            <xsl:text>.value()</xsl:text>
			</xsl:with-param>
                    </xsl:call-template>
                    </xsl:for-each>
                </xsl:when>
            </xsl:choose>
    </xsl:for-each>


    <xsl:for-each select="/compiz/plugin[@name=$pName]/descendant-or-self::option">
        <xsl:if test="@type = 'list' and ./desc/value and ./type/text() = 'int'">
            <xsl:text>
    m</xsl:text>
            <xsl:call-template name="printOptionName"/>
            <xsl:text>Mask = 0;
    foreach (CompOption::Value &amp;val, mOptions[</xsl:text>
            <xsl:call-template name="printOptionsEnumName"/>
            <xsl:text>].value ().list ())
        m</xsl:text>
            <xsl:call-template name="printOptionName"/>
        <xsl:text>Mask |= (1 &lt;&lt; val.i ());
</xsl:text>
        </xsl:if>
    </xsl:for-each>
        <xsl:text>
}

</xsl:text>
   </xsl:template>

       <xsl:template name="printDestructor">
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::~</xsl:text>
	<xsl:value-of select="$Plugin"/>
        <xsl:text>Options ()
{
}

</xsl:text>
    </xsl:template>

   <!-- String conversion helper functions -->

    <xsl:template name="print">
        <xsl:param name="text"/>
	<xsl:value-of select="translate($text,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
    </xsl:template>

    <xsl:template name="PRINT">
        <xsl:param name="text"/>
	<xsl:value-of select="translate($text,'abcdefghijklmnopqrstuvwxyz','ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
    </xsl:template>

    <xsl:template name="Print">
        <xsl:param name="text"/>
	<xsl:call-template name="PRINT">
	    <xsl:with-param name="text">
		<xsl:value-of select="substring($text,1,1)"/>
	    </xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="print">
	    <xsl:with-param name="text">
		<xsl:value-of select="substring($text,2)"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:template>

    <xsl:template name="PrintCamel">
        <xsl:param name="text"/>
        <xsl:variable name="textconv">
            <xsl:value-of select="translate($text,' +-/\','_____')"/>
        </xsl:variable>
	<xsl:if test="string-length($textconv)">
	    <xsl:if test="contains($textconv,'_')">
		<xsl:call-template name="Print">
		    <xsl:with-param name="text">
			<xsl:value-of select="substring-before($textconv,'_')"/>
		    </xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="PrintCamel">
		    <xsl:with-param name="text">
			<xsl:value-of select="substring-after($textconv,'_')"/>
		    </xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	    <xsl:if test="not(contains($textconv,'_'))">
	       <xsl:call-template name="Print">
		    <xsl:with-param name="text">
			<xsl:value-of select="$textconv"/>
		    </xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	</xsl:if>
    </xsl:template>

    <xsl:template name="saveCName">
        <xsl:param name="text"/>
        <xsl:variable name="textFirst">
            <xsl:value-of select="translate(substring($text,1,1),'0123456789 +-','zottffssen___')"/>
        </xsl:variable>
        <xsl:value-of select="concat($textFirst,substring($text,2))"/>
    </xsl:template>

<!-- Plugin name variables -->

    <xsl:variable name="pName">
        <xsl:value-of select="/compiz/plugin[@useBcop = 'true']/@name"/>
    </xsl:variable>

    <xsl:variable name="pCName">
        <xsl:call-template name="saveCName">
	    <xsl:with-param name="text">
		<xsl:value-of select="$pName"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:variable>

    <xsl:variable name="plugin">
        <xsl:call-template name="print">
	    <xsl:with-param name="text">
		<xsl:value-of select="$pCName"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:variable>

    <xsl:variable name="Plugin">
        <xsl:call-template name="Print">
	    <xsl:with-param name="text">
		<xsl:value-of select="$pCName"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:variable>

    <xsl:variable name="PLUGIN">
        <xsl:call-template name="PRINT">
	    <xsl:with-param name="text">
		<xsl:value-of select="$pCName"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:variable>

    <!-- global helper functions -->

    <xsl:template name="license">
<xsl:text>/*
 * This file is autogenerated with bcop:
 * The Compiz option code generator
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

</xsl:text>
    </xsl:template>

    <xsl:template name="printOptionsEnumName">
        <xsl:value-of select="$Plugin"/>
        <xsl:text>Options::</xsl:text>
        <xsl:call-template name="PrintCamel">
	    <xsl:with-param name="text">
		<xsl:value-of select="@name"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:template>

    <xsl:template name="printOptionsEnumNamePure">
        <xsl:call-template name="PrintCamel">
	    <xsl:with-param name="text">
		<xsl:value-of select="@name"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:template>

    <xsl:template name="printOptionName">
        <xsl:call-template name="PrintCamel">
	    <xsl:with-param name="text">
		<xsl:value-of select="@name"/>
	    </xsl:with-param>
	</xsl:call-template>
    </xsl:template>

</xsl:stylesheet>
