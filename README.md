# XCatLock — Protect keyboard from cat

XCatLock is an X11 program that grabs all keyboard and mouse input, and only 
ungrabs it after a correct passphrase (specified as argument) is entered. It 
is different from a screen locker in that it leaves the screen visible and 
only prevents key and mouse presses from going to other applications.

The use case is being able to keep looking at the screen while a cat is 
walking or sitting on the keyboard. That's the reason for the name.

## Limitations
- XCatLock **will not work with Wayland**. It probably will open, but since
Wayland isolates all X11 applications into an XWayland server, it will not 
be able to grab *all* the keyboard and mouse input.

- Switching to another framebuffer using Ctrl+Alt+F<num> is still possible, at
least on Linux. This is due to the Ctrl+Alt+F<num> key combination being 
handled directly by the Linux kernel. XCatLock grabbing and intercepting all
X11 keyboard events does not stop the kernel from reacting to the key 
combination in its own way. See this link for more detailed explanation of 
what happens when Ctrl+Alt+F<num> is pressed: 
https://unix.stackexchange.com/questions/63688/what-happens-when-ctrl-alt-fnum-is-pressed/63700#63700
