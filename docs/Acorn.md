# Acorn Operating System

## Abstract

Acorn is a new graphical and distributed research operating system that runs on
top of the WebAssembly stack. It could be thought of a re-invention of inferno™
operating system from Bell Labs but using WASM (abbreviation of WebAssembly)
as the Virtual Machine (VM) instead of [Dis](http://www.vitanuova.com/inferno/papers/dis.html).

## Motivation

Today our data is tied to our computer hardware in the same way it was in the
80s. We have a personal notebook, a personal mobile phone, a company owned
notebook and maybe even a company owned mobile phone. Each of those devices has
local storages with several hundred gigabytes of data. A big portion of the
data on the machine's disk is the operating system base programs and they are
mostly duplicated (the program calc, and hundreds of others, exists in both
your personal and company notebooks). And talking about your own data, you need
to carry your notebook everywhere you need to access your files. As a solution
to this problem, some people put their data on the cloud in solutions like
Google Drive or Dropbox but then they have to deal with upload/download of data
on multiple devices and trust the provider that it does not see their files.

Now think for yourself:

What if you could power up any computer (even the notebook of your mother) and
see your files and the exact state of your personal computer in the last time
you used it? And this without having to download everything! What if all this
can be achieved without resorting to the cloud?

What if you could upgrade your software on one device and they be ready on all others?

What if you get a ransomware malware but there’s no data in the laptop to be
destroyed? What if the file server has daily archival of your data? What if you
could restore your files from past archives easily?

That’s some of the things envisioned by Acorn OS.

The web is here to stay, then any new operating system should at least have a
browser otherwise users could not adopt. The web stack is huge, bad designed
and hard to implement properly because there are tons of standards and several
deviations from browser vendors. The size of a web browser code base is close
to the size of some operating systems.

Plan9 is a full-featured operating system built from scratch with those ideas
implemented in the 90s but did not get traction in user adoption and the most
reason was the lack of a modern web browser. Because of the size of the web
stack, porting a browser to plan9 was so challenging that plan9 developers
felt it was easier to port plan9 userspace to other operating systems.

Userspace plan9 (plan9port) runs as processes on a host UNIX-based operating
system (Linux, FreeBSD, OSX and so on) and they don’t improve much on UNIX
problems plan9 was designed to solve.

Inferno™ runs hosted on several operating systems, including on non-UNIX like
Windows but it’s not maintained anymore, lacks a web browser and requires the
use of Limbo Programming Language. Then we could not use it to develop software
to the current industry.

Acorn is an attempt to solve the following:

* Have a programming environment that works seamlessly everywhere (from cloud
to notebooks, from Linux to Windows).
* Separate your data from your computer. Your data must be available everywhere
from your notebook to your mobile phone, a server or your friend’s computer.
* Resurrect the everything is a file concept.
* Simple programming interfaces (easy to write graphical or distributed apps)
* Have a modern browser, that means you could use Acorn browser instead of open
chrome/firefox
* Be network distributed, in the sense that multiple Acorn servers provides the
set of visible resources in each terminal.
* Interoperate and extend current technologies (Containers, Kubernetes, etc)

## Entering the System

Below is a proposal for an (initially text-based) user interface for Acorn
independently if it’s used as a guest OS or bare metal.
When started, either by booting up the machine from the disk[b][c] or by
starting the `acorn` program from the host OS, it must present the interface
below:

```
Welcome to Acorn Operating System!
Loading drivers:
    [x] diskfs
    [x] netfs
    [x] kbdfs
    [x] mousefs
    [x] screenfs
    [x] camfs
Root is from (local, tcp)[local]:
```

After basic drivers loaded the system will ask for where the root filesystem
should be loaded. Choosing local will make Acorn to use the machine storage,
ie, the system will ask the user for the right disk device (in a bare metal
install) or ask to the host path directory if in guest mode.
In case of choosing tcp the system will ask for the file server address. In
case of using a remote file server, the terminal could be diskless.

```
Root is from (local, tcp)[local]: Return
Set workspace in host filesystem [/home/i4k/acorn]: Return
[diskfs] /home/i4k/acorn mounted on /
Starting userspace:
User [i4k]: 

```

After mounting the root filesystem the system will prompt for the username that
owns this session at the terminal. You could give any name, Acorn trusts you.
Terminals are not shared by multiple users at the same time but a file server
is.

```
User[i4k]: Return
Loading network config from /lib/ndb/local
Auth: not found
File server: not found
λ>
```

The file /lib/ndb/local is the network database. As this is the first time you
log into the system, there’s no authentication nor file server configured
there. In case you had it properly set up you would see something like:

```
User[i4k]: Return
Loading network config from /lib/ndb/local
office:
    Auth: auth.acorn.acme.com
    File server: fs.acorn.acme.com
community:
    Auth: auth.community.acorn-os.io
    File server: fs.community.acorn-os.io
Authenticate on (office, community, all, none)[office]: Return
Authenticating on [office]
User[i4k]: i4k
Password: ***************
Ok
File server [office] mounted on /n/office
λ>
```

In this case, the system found two remote infrastructures (office and
community) and ask the user if he/she wants to authenticate on some of them.
The user is free to ignore and just log in using local files. When
authenticating the user must provide valid credentials in the remote
infrastructure. There are several benefits of using a remote file server but
this will be discussed extensively in the next section.

The symbol λ> is the shell prompt. Now you can type commands into the system
but Acorn is better with the graphical user interface (GUI). You can start the
window manager typing:


```
λ> win/wm
```

The command above will redraw the screen and present you a Desktop area.


## Acorn Concepts


Using Chrome as the VM platform we can leverage the hardware access already
built into browsers exposing them as high-level device drivers into Acorn. That
means Acorn could support webcams, for example, without having to maintain a
device driver for each camera manufacturer, and the same for everything that
the browser and NodeJS have APIs (disk, memory, GPU[d][e], screen, mouse,
keyboard, and so on).

Acorn is standalone and self-contained, that means all your work could be made
from it, without requiring you to open external software like a web browser or
a Linux/Windows/OSX terminal.

But you can deploy an Acorn service on the cloud and then load the resources
there in your current namespace to use the remote disk or CPU but rendering the
UI locally.

Every resource mapped from the host to Acorn should interface by memory access
(not using network or disk) for performance reasons.

Every device driver of Acorn must expose a file system interface using a variant
of the 9p/Styx protocol.

Acorn will not run any process by default but just load all kernel device
drivers (the ones that require access to the shim layers).

Acorn doesn’t have a pseudoterminal (pty) driver but one could be created on
top of the screenfs driver.
The first program invoked must mount the screenfs driver otherwise it doesn’t
have any place to print output.

## Screenfs

Screenfs supports a Vector and Raster interface.

```
/dev/screen/vector2d
/dev/screen/vector3d
/dev/screen/raster
```

When open /dev/screen/vector2d you get a context for a 2D canvas area in the
browser. And then you could issue commands like “fillRect(25, 25, 100, 100)” to
fill a rectangle at pos x=25, y=25 with width=100 and height=100. Same browser
API defined here.

When using the raster interface you should handle the draw yourself and just
write to the /dev/screen/raster/primary. You could use several offscreen
renders as well in `/dev/screen/raster/offscreen`.

The Diskfs is just a wrapper to the host disk.
