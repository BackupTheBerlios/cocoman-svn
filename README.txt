Installation
------------
These instructions make some assumptions about how the system is to be set up. The user that our build system is running as is builduser. The user the web server that's serving the interface is running as, is webuser. The web server is also used by untrusted users so the build system needs to be running as a seperate user and there needs to be careful communication between the two. If your set up is different, these directions might not exactly apply to you.

1. Extract the tarball or whatever to a directory that is accessible by both builduser and webuser-data. It shouldn't matter if this directory is servable, but for safety's sake we recommend it is not. All of the extracted files should be owned by builduser. Paths in the rest of these instructions are relative to where you extracted the program to.
. Create a directory for each problem in problems/ of the form Problem1, Problem2, etc. They are case sensitive.
. In each of those directories put the test cases. The test cases each consist of a pair a files: <test case number>.input and <test case number>.output. There must be at least one test case for each problem.
. Also create Problem1, Problem2, etc directories in submissions/
. mkdir logs/compiles
. touch manager/conf/users.txt
. touch logs/log
. build mydiff (TODO)

. "chmod -R g-r-w-x *"
. "chmod -R o-r-w-x *".
. chmod o+r+x conf
. "chmod o+r manager/conf/users.txt"
. chmod o+r conf/times.txt
. "chmod o+r+x logs"
. chmod 644 logs/log
. "chmod o+r+w+x temp_web"
. "chmod o+r+x web"
. "chmod o+r web/*"

. Edit manager/conf/contest.conf. You almost certainly will need to change ROOT, and should look at at least JAVABIN, ALLOWABLE_IPS, and NUM_PROBLEMS.
. Edit manager/conf/times.txt. It should contain 2 lines. The first is the start time of the contest in 24 hour format. The second is the length of the contest. Both times are hours:minutes:seconds.
. Edit web/config.inc and set $contest_root to the same thing as ROOT from before.
. Take a look at manager/conf/users.txt and make sure there are no entries that you don't want.
. Create a symlink somewhere in a web serverable directory pointing to the web/ folder. If you can't make a symlink in the servable area that points out of it, you can copy or move web/ into your servable area instead.
. cd to "manager/". Run "./process_registration".
. Run "./manager"
. Be sure that if you add files (or directories) later such as test cases, that their permissions are set so that they are only accessible by acmweb unless you're sure they need to be available to other users.

Notes
-----
leaving conf/times.txt blank indicates contest hasn't started yet
All user ids have to be the same length.
The directories in problems/ should be of the form "Problem1", "Problem2", etc.
