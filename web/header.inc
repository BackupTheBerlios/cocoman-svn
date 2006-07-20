<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html>
<head>
  <title>ACM Coding Contest Scoreboard</title>
  <link rel="stylesheet" type="text/css" href="style.css" />

  <!-- from: http://grizzlyweb.com/webmaster/javascripts/refresh.asp 
  Define multiple JavaScript code blocks, where each JavaScript version block redefines the same function.  Only the last version block that is supported by the browser will be used. -->
  <noscript>
  <!--
      We have the "refresh" meta-tag in case the user's browser does
      not correctly support JavaScript or has JavaScript disabled.
  -->
  <meta http-equiv="refresh" content="10" />
  
  </noscript>
  
  <script language="JavaScript">
  <!--
  var sURL = unescape(window.location.pathname);
  
  var refreshActive = true;
  function checkRefresh()
  {
      // Allows refreshing to be disabled while a user is browsing for a file
      if (refreshActive) {
          refresh();
      }
  }
  
  function doLoad()
  {
      setTimeout("checkRefresh()", 10 * 1000);
  }
  
  function refresh()
  {
      //  This version of the refresh function will cause a new
      //  entry in the visitor's history.  It is provided for
      //  those browsers that only support JavaScript 1.0.
      window.location.href = sURL;
  }
  
  function clickedBrowse()
  {
      refreshActive = false;
      document.getElementById('refreshStatus').innerHTML = "<i>Auto-refreshing disabled</i>";
  }
  //-->
  </script>
  
  <script language="JavaScript1.1">
  <!--
  function refresh()
  {
      //  This version does NOT cause an entry in the browser's
      //  page view history.  Most browsers will always retrieve
      //  the document from the web-server whether it is already
      //  in the browsers page-cache or not.
      window.location.replace(sURL);
  }
  //-->
  </script>
  
  <script language="JavaScript1.2">
  <!--
  function refresh()
  {
      //  This version of the refresh function will be invoked
      //  for browsers that support JavaScript version 1.2
      //
      //  The argument to the location.reload function determines
      //  if the browser should retrieve the document from the
      //  web-server.  In our example all we need to do is cause
      //  the JavaScript block in the document body to be
      //  re-evaluated.  If we needed to pull the document from
      //  the web-server again (such as where the document contents
      //  change dynamically) we would pass the argument as 'true'.
      window.location.reload(true);
  }
  //-->
  </script>
</head>

<body id="document" onload="doLoad()">
