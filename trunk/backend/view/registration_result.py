template = \
"""
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html>
<head>
  <title>ACM Coding Contest Submission</title>
  <link rel="stylesheet" type="text/css" href="style.css" />
</head>

<body>
  <p>
    %s
  </p>
  <p>
    %s
  </p>
  <hr />
  
  <p>
    <a href="http://validator.w3.org/check?uri=referer"><img
        src="http://www.w3.org/Icons/valid-xhtml10"
        alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a>
  </p>

</body>
</html>
"""

def failed_html(message):
    return template % (message, '')

def succeeded_html(message, id):
    link_to_scoreboard = \
"""
After you have written down your User ID, 
<a href="scoreboard.php?id=%s">click here to continue</a>.
""" % id

    return template % (message, link_to_scoreboard)
