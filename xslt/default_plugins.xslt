<!--
  Copyright © 2009 Dennis Kasprzyk

  Permission to use, copy, modify, distribute, and sell this software
  and its documentation for any purpose is hereby granted without
  fee, provided that the above copyright notice appear in all copies
  and that both that copyright notice and this permission notice
  appear in supporting documentation, and that the name of
  Dennis Kasprzyk not be used in advertising or publicity pertaining to
  distribution of the software without specific, written prior permission.
  Dennis Kasprzyk makes no representations about the suitability of this
  software for any purpose. It is provided "as is" without express or
  implied warranty.

  DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
  NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
  OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Authors: Dennis Kasprzyk <onestone@compiz.org>
-->

<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' >
  <xsl:output method="xml" indent="yes"/>


  <xsl:template match="default_plugins">
    <default>
    <xsl:call-template name="processList">
      <xsl:with-param name="list">
        <xsl:value-of select="$default_plugins"/>
      </xsl:with-param>
    </xsl:call-template>
    </default>
  </xsl:template>

  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template name="processList">
    <xsl:param name="list"/>
    <xsl:call-template name="doValue">
      <xsl:with-param name="list">
        <xsl:value-of select="normalize-space(translate($list,',;\','   '))"/>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="doValue">
    <xsl:param name="list"/>
    <xsl:if test="string-length($list) > 0">
      <value>
        <xsl:choose>
          <xsl:when test="string-length(substring-before($list,' ')) > 0">
            <xsl:value-of select="substring-before($list,' ')"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$list"/>
          </xsl:otherwise>
        </xsl:choose>
      </value>
      <xsl:call-template name="doValue">
        <xsl:with-param name="list">
          <xsl:value-of select="substring-after($list,' ')"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
