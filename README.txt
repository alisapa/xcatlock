NAME
       xcatlock - protect keyboard from cat


SYNOPSIS
       xcatlock [-test] [-tiny] [-verbose] passphrase


DESCRIPTION
       XCatLock is an X11 program that grabs all keyboard and mouse input, and
       only ungrabs it after a correct passphrase (specified as  argument)  is
       entered.  It  is  different  from a screen locker in that it leaves the
       screen visible and only prevents key and mouse presses  from  going  to
       other applications.

       The use case is being able to keep looking at the screen while a cat is
       walking or sitting on the keyboard. That's the reason for the name.


OPTIONS
       -test  If this option is set, the program will  exit  as  soon  as  any
              mouse button is pressed. This is useful for testing a passphrase
              while having an easy method to exit  the  program  if  something
              went wrong.

              The lock icon will appear grey to indicate testing mode.

       -tiny  Causes the created window to be as small as possible (1x1 pixel)
              and undecorated, making it almost invisible.

       -verbose
              Causes the program to print  a  lot  of  output  about  received
              events and program state to standard output. Only useful for de‐
              bugging.


NOTES
       Do not use your password as the passphrase.
              Besides the fact that all commands entered in a shell  are  usu‐
              ally  stored in plain-text in a history file, that would also be
              a bad idea because this program takes no precautions to keep the
              passphrase safe, and because there is no limit on incorrect "at‐
              tempts" or keypresses.

              XCatLock unlocking with a given passphrase instead of the user's
              password  is  intentional.  Many systems have a limit set on in‐
              correct password attempts, after which the account  gets  tempo‐
              rarily locked. A cat walking or sitting on the keyboard can gen‐
              erate many incorrect attempts if it happens to touch  the  Enter
              key.


       XCatLock will not work with Wayland.
              It probably will open, but since Wayland isolates all X11 appli‐
              cations into an XWayland server, it will not be able to grab all
              the keyboard and mouse input.


       XCatLock can't stop key combinations handled by the kernel.
              On Linux, switching to another framebuffer using Ctrl+Alt+F<num>
              is still possible, as is the use of PrintScr+SysRq.  This is be‐
              cause  these  key combinations are handled directly by the Linux
              kernel.  XCatLock grabbing and  intercepting  all  X11  keyboard
              events  does not stop the kernel from reacting to the key combi‐
              nation in its own way.  See this link for more detailed explana‐
              tion   of   what   happens   when  Ctrl+Alt+F<num>  is  pressed:
              https://unix.stackexchange.com/questions/63688/what-happens-
              when-ctrl-alt-fnum-is-pressed/63700#63700


       If using Unicode characters, first test the passphrase with -test.
              XCatLock  uses XIM for processing keyboard input, and in princi‐
              ple supports UTF8.  The author of the program  has  successfully
              tested  it  with  German  and Russian keyboard layouts on FCITX.
              However, when using FCITX Hangul layout, instead of Korean char‐
              acters it reads English characters.  Many more input methods and
              layouts haven't been tested at all.  So, to avoid  the  inconve‐
              nience  of having to close a program that grabs all keyboard in‐
              put, if using a keyboard layout or passphrase with Unicode char‐
              acters, first test it with -test.



                                  2023-05-29                       XCATLOCK(1)
