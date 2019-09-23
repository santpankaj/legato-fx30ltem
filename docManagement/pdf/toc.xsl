<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:outline="http://wkhtmltopdf.org/outline"
                xmlns="http://www.w3.org/1999/xhtml">
  <xsl:output doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
              indent="yes" />
  <xsl:template match="outline:outline">
    <html>
      <head>
        <title>Contents</title>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <style>
         .toc-xsl h1 {
            text-align: center;
            font-size: 16px;
            font-family: 'Open Sans', arial;
          }
          .toc-xsl div {border-bottom: 1px dashed #CCC;}
          .toc-xsl span {float: right;}
          .toc-xsl li {list-style: none;}
          .toc-xsl ul {
          color:#7395A0;
             font-size: 16px;
            font-family: 'Open Sans', arial;
          }
          .toc-xsl ul ul {font-size: 95%; font-weight:normal; border-left: 2px solid #EEE; }
          .toc-xsl ul {padding-left: 0em; font-weight:bolder; }
          .toc-xsl ul ul {padding-left: 0.5em; margin: 0.2em 0em 0.2em 0.5em;}
          .toc-xsl a {text-decoration:none;  color:#00677A; font-family: 'Open Sans', arial;}
        </style>
      </head>
      <body class="toc-xsl">
        <h1>Contents</h1>
        <ul><xsl:apply-templates select="outline:item/outline:item"/></ul>
      </body>
    </html>
  </xsl:template>
  <xsl:template match="outline:item">
    <li>
      <xsl:if test="@title!=''">
        <div>
          <a>
            <xsl:if test="@link">
              <xsl:attribute name="href"><xsl:value-of select="@link"/></xsl:attribute>
            </xsl:if>
            <xsl:if test="@backLink">
              <xsl:attribute name="name"><xsl:value-of select="@backLink"/></xsl:attribute>
            </xsl:if>
            <xsl:value-of select="@title" />
          </a>
          <span> <xsl:value-of select="@page" /> </span>
        </div>
      </xsl:if>
      <ul>
        <xsl:comment>added to prevent self-closing tags in QtXmlPatterns</xsl:comment>
        <xsl:apply-templates select="outline:item"/>
      </ul>
    </li>
  </xsl:template>
</xsl:stylesheet>
