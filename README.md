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

Note that due to the way I use the extended ascii character set to display menu borders and shading,
it is not portable to windows systems. (It’s not a bug, it’s a feature!)

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

Use the `-c` option to override the default menu file path:
```
bmenu -c /path/to/menu/file
```

Use the `-t` option to override the default menu prompt:
```
bmenu -t 'Choose an Option:'
```

Set Up B-Menu as a Login Manager
--------------------------------

To run b-menu when you login, place the following code in `$HOME/.bash_profile`:

```
if [[ "$(tty)" == '/dev/tty1' ]]; then                                                                        
    [[ -n "$BMENU_SPAWN" ]] && return                                                                         
    export BMENU_SPAWN=1                                                                                      
    # Avoid executing bmenu when X11 has already been started.                                                
    [[ -z "$DISPLAY$SSH_TTY$(pgrep xinit)" ]] && exec bmenu                                                   
fi
```

If you do this, it is best to unclude a menu option to launch a shell. Add the following to your menu
options file:

```
Shell:/bin/bash --login
```
