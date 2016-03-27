B-Menu
======

![Screenshot](http://i.imgur.com/QV5Ra8V.png)

What is B-Menu?
---------------

B-Menu is a minimalistic general purpose terminal menu written in C. It was designed to replace
terminal-based login managers such as CDM. However, it can easily serve as a simple terminal menu for
any situation where you would like to execute a command from a set of menu selections.

B-Menu has zero dependencies, and it is intentionally feature-minimal. This keeps it easy to install, 
configure, and run. Plus, who really needs all those extra features anyway? Don't you just want to display
a menu and execute a command? Me too! B-Menu is for you!

I designed B-Menu to look like it uses ncurses, even though it doesn't actually use it. I love the
way ncurses looks but didn't want the added overhead. For a simple menu such as this, it really wasn't
necessary.

License
-------

This program is free software; you can redistribute it and/or modify it under the terms of the GNU 
General Public License as published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.  See [COPYING](COPYING) for more details.

Installation
------------
```
git clone https://github.com/bartobri/bmenu.git
cd ./bmenu/src
make
```

This will create the executable file `bmenu`. From here you can copy it to wherever you like. Typically
this would be /usr/bin.

Configuration
-------------

You can configure B-Menu by editing and/or creating $HOME/.bmenu. The file should consist of pairs of
menu titles and commands, one pair per line. The pair should be seperated by a colon. below is an example:

```
Clear Screen:/usr/bin/clear
Dir Listing:/usr/bin/ls -l
```
