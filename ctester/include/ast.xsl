<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
  <html>
  <body>
  <link rel="stylesheet" type="text/css" href="include/ast.css" />
  <xsl:for-each select="AsirikuyPortfolio/AsirikuyBacktest">

  <h2><xsl:value-of select="Header/SystemName"/>&#160;<xsl:value-of select="Symbol"/></h2>
  <span class="bold">Timeframe: </span> <xsl:value-of select="Header/TimeFrame"/><br />
  <span class="bold">Start: </span> <xsl:value-of select="Header/PeriodStart"/><br />
  <span class="bold">End: </span> <xsl:value-of select="Header/PeriodEnd"/><br />
  <span class="bold">Risk: </span> <xsl:value-of select="Header/RiskPercent"/><br />
  <span class="bold">Deposit: </span> <xsl:value-of select="Header/Deposit"/><br />
    <table id="trades">
      <thead>
      	<th>No</th>
      	<th>Time</th>
      	<th>Type</th>
      	<th>OrderID</th>
      	<th>Size</th>
      	<th>Price</th>
      	<th>SL</th>
      	<th>TP</th>
      	<th>Profit</th>
      	<th>Balance</th>
      </thead>
      <xsl:for-each select="Trades/Trade">
      <tr>
        <td><xsl:value-of select="No"/></td>
        <td><xsl:value-of select="Time"/></td>
        <td><xsl:value-of select="Type"/></td>
        <td><xsl:value-of select="OrderID"/></td>
        <td><xsl:value-of select="Size"/></td>
        <td><xsl:value-of select="Price"/></td>
        <td><xsl:value-of select="SL"/></td>
        <td><xsl:value-of select="TP"/></td>
        <td><xsl:value-of select="Profit"/></td>
        <td><xsl:value-of select="Balance"/></td>
      </tr>
      </xsl:for-each>
    </table>
  </xsl:for-each>
  </body>
  </html>
</xsl:template>
</xsl:stylesheet>