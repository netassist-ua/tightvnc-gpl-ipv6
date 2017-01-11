# TightVNC IPv6

TightVNC is a free remote control software package. With TightVNC, you can see the desktop of a remote machine and control it with your local mouse and keyboard, just like you would do it sitting in the front of that computer. 

TightVNC is:

* free for both personal and commercial usage, with full source code available,
* useful in administration, tech support, education, and for many other purposes,
* cross-platform, available for Windows and Unix, with Java client included,
* compatible with standard VNC software, conforming to RFB protocol specifications

# IPv6

This fork aim on adding IPv6 support into TightVNC Server and TightVNC Client.
In this version only IPv6 socket listening  implemented yet.

IPv4 clients should be able to work with IPv6 server using IPv4-mapped addresses like ::FFFF:255.255.255.255.
Windows XP does not support IPV4Mapped, so application works with Windows Vista+.

