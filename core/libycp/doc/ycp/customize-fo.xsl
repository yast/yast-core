<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/fo/docbook.xsl"/>

  <!-- Set paper size to A4 -->
  <xsl:param name="paper.type" select="'A4'"/>

  <!-- draft mode? -->
  <xsl:param name="draft.mode" select="'yes'"/>
  <!-- fop does not handle pngs, so don't use an image -->
  <xsl:param name="draft.watermark.image" select="''"></xsl:param>

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
  <xsl:param name="admon.graphics.path">html/images/admon/</xsl:param>

  <!-- Index generation does not work - let's disable for now -->
  <xsl:param name="generate.index" select="0"></xsl:param>

</xsl:stylesheet>
