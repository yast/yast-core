<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/fo/docbook.xsl"/>

  <!-- Set paper size to A4 -->
  <xsl:param name="paper.type" select="'A4'"/>

  <!-- draft mode? -->
  <xsl:param name="draft.mode" select="'no'"/>
  <!-- fop does not handle pngs, so don't use an image -->
  <!--
  <xsl:param name="draft.watermark.image" select="''"></xsl:param>
  -->

  <!--- Number sections -->
  <xsl:param name="section.autolabel" select="1"/>
  <xsl:param name="section.label.includes.component.label" select="1"/>

  <!-- add sections to toc up to depth 5 -->
  <xsl:param name="toc.section.depth">5</xsl:param>

  <!-- Use shade for verbatim environments -->
  <xsl:param name="shade.verbatim" select="1"></xsl:param>

  <!-- enable bookmarks -->
  <xsl:param name="fop.extensions" select="1"></xsl:param>

  <!-- double sided printing -->
  <xsl:param name="double.sided" select="1"></xsl:param>

  <!-- use custom icons for admonition -->
  <xsl:param name="admon.graphics" select="1"/>
  <xsl:param name="admon.graphics.extension" select="'.gif'"></xsl:param>
  <xsl:param name="admon.graphics.path">html/images/</xsl:param>

  <!-- Index generation does not work - let's disable for now -->
  <xsl:param name="generate.index" select="0"></xsl:param>

<xsl:attribute-set name="section.title.properties">
  <xsl:attribute name="font-family">
    <xsl:value-of select="$title.font.family"/>
  </xsl:attribute>
  <xsl:attribute name="font-weight">bold</xsl:attribute>
  <!-- font size is calculated dynamically by section.heading template -->
  <xsl:attribute name="keep-with-next.within-column">always</xsl:attribute>
  <xsl:attribute name="space-before.minimum">0.8em</xsl:attribute>
  <xsl:attribute name="space-before.optimum">1.0em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">1.2em</xsl:attribute>
</xsl:attribute-set>


<xsl:attribute-set name="section.title.level1.properties">
  <xsl:attribute name="font-size">
    <xsl:value-of select="$body.font.master * 1.8"/>
    <xsl:text>pt</xsl:text>
  </xsl:attribute>
</xsl:attribute-set>

<xsl:attribute-set name="section.title.level2.properties">
  <xsl:attribute name="font-size">16pt</xsl:attribute>
</xsl:attribute-set>

<!--
<xsl:attribute-set name="section.level1.properties">
  <xsl:attribute name="break-before">page</xsl:attribute>
</xsl:attribute-set>
-->

<!-- Programlisting -->
<xsl:attribute-set name="monospace.verbatim.properties"
use-attribute-sets="verbatim.properties">
  <xsl:attribute name="font-family">
    <xsl:value-of select="$monospace.font.family"/>
  </xsl:attribute>
  <xsl:attribute name="font-size">
    <xsl:value-of select="$body.font.master * 0.5"/>
    <xsl:text>pt</xsl:text>
  </xsl:attribute>
<!--
  <xsl:attribute name="border-color">#0000FF</xsl:attribute>
  <xsl:attribute name="border-style">solid</xsl:attribute>
  <xsl:attribute name="border-width">heavy</xsl:attribute>
-->
  <xsl:attribute name="background-color">#F0F0F0</xsl:attribute>
</xsl:attribute-set>


<xsl:template match="processing-instruction('anas-pagebreak')">
    <fo:block xmlns:fo="http://www.w3.org/1999/XSL/Format" break-before='page'/>
</xsl:template>

</xsl:stylesheet>
