<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

   <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl"/>

  <!-- Output directory -->
  <xsl:param name="base.dir" select="'html/'"/>

  <!--- Number sections -->
  <xsl:param name="section.autolabel" select="1"/>
  <xsl:param name="section.label.includes.component.label" select="1"/>

  <!-- add sections to toc up to depth 5 -->
  <xsl:param name="toc.section.depth">5</xsl:param>

  <!-- use ids as filename instead of numbers -->
  <xsl:param name="use.id.as.filename" select="'1'"></xsl:param>

  <!-- draft mode? 
  <xsl:param name="draft.mode" select="'no'"></xsl:param>
  <xsl:param name="draft.watermark.image" select="'images/draft.png'"></xsl:param>
  -->
<xsl:param name="shade.verbatim" select="1"/>
<xsl:attribute-set name="shade.verbatim.style">
  <xsl:attribute name="border">0</xsl:attribute>
  <xsl:attribute name="width">100%</xsl:attribute>
  <xsl:attribute name="bgcolor">#E0E0E0</xsl:attribute>
</xsl:attribute-set>

  <xsl:param name="navig.showtitles">1</xsl:param>
  <xsl:param name="html.extra.head.links" select="1"></xsl:param>
  <xsl:param name="generate.legalnotice.link" select="0"/>

  <!-- use custom icons for navigation -->
  <xsl:param name="navig.graphics" select="1"/>
  <xsl:param name="navig.graphics.path">images/navig/</xsl:param>
  <xsl:param name="navig.graphics.extension" select="'.png'"/>

  <!-- use custom icons for admonition -->
  <xsl:param name="admon.graphics" select="1"/>
  <xsl:param name="admon.graphics.path">images/</xsl:param>
  <xsl:param name="admon.graphic.width" select="32"/>
  <xsl:param name="admon.style" select="''"/>

  <!-- Use shade for verbatim environments -->
  <xsl:param name="shade.verbatim" select="1"></xsl:param>

  <!-- Use own stylesheet, adopted version of susebook.css -->
  <!-- I'm not sure whether this is really an improvement -->
  <xsl:param name="html.stylesheet">susebooks.css</xsl:param>
  <xsl:param name="header.rule" select="0"></xsl:param>
  <xsl:param name="footer.rule" select="0"></xsl:param>

  <!-- Show revisionflag -->
  <xsl:param name="show.revisionflag">1</xsl:param>

  <!-- shut up!  -->
  <xsl:param name="chunk.quietly" select="1"></xsl:param>

  <!-- include header.xsl -->
  <!--
  <xsl:include href="header.xsl" />
  -->


</xsl:stylesheet>
