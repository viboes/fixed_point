This directory contains the file "boost-build.jam" which search for a Boost install as follows:

* In a directory specified by the --boost=path command line option.
* In a directory specified by the BOOST environment variable.
* In the directories ../boost and ../Trunk.

In addition the variables BOOST and BOOST_ROOT are set to point to the root of the Boost install, so to refer to other
Boost libraries and the main Boost headers, your Jamfile should contain something like:

import modules ;

local boost-path = [ modules.peek : BOOST ] ;

And then you can refer to a Boost library "foo" as:

$(boost-path)/libs/foo/build//boost_foo

Note that if your project does not specify a Jamroot file, then a default one is provided for you,
and that this file will automatically add $(BOOST)/ to your include path.



