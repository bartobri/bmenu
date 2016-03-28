B-Menu
======

![Screenshot](http://i.imgur.com/QV5Ra8V.png)

What is B-Menu?
---------------

B-Menu is a minimalistic general purpose terminal menu written in C. It was designed to replace
terminal-based login managers such as CDM. However, it can easily serve as a simple terminal menu for
any situation where you would like to execute a command from a set of menu selections.

B-Menu has zero dependencies, and it is intentionally feature-minimal. This keeps it easy to install, 
configure, and run. 

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

This will create the executable file `bmenu`. Most users will want to copy it to /usr/bin, but you can
place it anywhere you like.

Configuring The Menu
--------------------

By default, b-menu looks for menu options in $HOME/.bmenu (use `-c` to override,  see below). This file
should consist of one menu option and one command on each line, seperated by a colon.

Example:

```
Clear Screen:/usr/bin/clear
Dir Listing:/usr/bin/ls -l
```

Command Line Options
--------------------

### -c 

Use the `-c` option to override the default menu file path:
```
bmenu -c /path/to/menu/file
```

### -t

Use the `-t` option to override the default menu prompt:
```
bmenu -c 'Choose an Option:'
```

Set Up B-Menu as a Login Manager
--------------------------------


