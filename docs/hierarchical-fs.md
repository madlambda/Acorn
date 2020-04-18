# The Fundamental Theorem of Hierarchical File Systems

## Introduction

The concept of a file is the same (or very close) on most operating systems of
today. The content of a file is a one-dimensional byte array where the meaning
of its content comes from the reader's interpretation. The basic file
operations are: create, open, read, write, seek and close. That simplicity has
a huge impact on the success of Multics and Unix, and it still benefits today's
software. On Unix based operating systems, the input and output of programs are
(special) files. This way, the output of one program (the standard output file
or stdout) could be piped into the input (the standard input file or stdin) of
another to solve complex tasks by just using byte manipulation tools. For this
to work, a special kind of file, called pipe, and a convention (eg.: read from
stdin, write to stdout, configurations from arguments, seeks are not allowed or
ignored, and so on) must be in place:

If the file is on disk, then the time to sequentially read 1 MB of its content
is around 20ms (or 150 us from an SSD) and if the file is a pipe, then the same
read could spend 250 us. But if the file is remote then the time goes to a few
seconds.[1] As the file content does not have an internal structure, a program
that needs to retrieve some specific information inside it needs to read all
its content or in case of having a header, at least some few blocks until it
could seek to the information.

Most files that need to be consumed only by computer programs have a file
format to ease the process of traversing its internal data. One way to work
around that problem is to split the resource information into several files
inside a folder. That way, specific fields could be directly addressed but the
file layout become hard to maintain. As an example, think about a user’s detail
information modeled on several files:

```
$ ls /users/1
id firstname surname birthday profession country state address zip
```

If the resource data has hundreds of fields as is the case for some real world
applications that have to deal with REST APIs that reply with JSON/XML composed
of dozens of fields, this idea doesn't work well.

Modern software that communicates on the internet commonly does not share files
but uses general protocols (HTTP/1/2/3, GRPC and so on) that support a way to
express the semantics of the communication and scales well. Eg.: When using
HTTP, the client software could express the intention of requesting the name
and email of the user with id equals 10 using the GET method (or verb):

```
GET /users/10?fields=name,email
```

That way, even if the server store thousand users and even if a user is
comprised of hundred fields, the server could comprehend the client’s intention
and reply with just name and email, saving a lot of bandwidth and time.

The file interface is heavy used in the design of most operating systems when a
specialized syscall is not required (procfs, cgroups, and so on) but rare in
the design of distributed software.

The Plan9 Operating System introduced the 9P protocol, a file abstraction with
a one-to-one network protocol based on remote procedure calls. This made
possible programs to become file servers that could communicate by means of
virtual file operations. This novel idea solves elegantly the inter-process
communication throughout the system by providing a simple uniform interface
which programs and kernel can use to communicate.

The Plan9 approach fall short when it comes to modern application requirements.
Some file servers document its own format that clients must parse in order to
retrieve specific information. For example, from the manual of mouse(3):

```
Reading the mouse file returns the mouse
status: its position and button state.

The read blocks until the state has
changed since the last read.  The read returns 49 bytes: the
letter m followed by four decimal strings, each 11 charac-
ters wide followed by a blank: x and y, coordinates of the
mouse position in the screen image; buttons, a bitmask with
the 1, 2, and 4 bits set when the mouse's left, middle, and
right buttons, respectively, are down; and msec, a timestamp,
in units of milliseconds.
```

A client interested in the mouse pointer must read the /dev/mousectl, parse its
content and convert character strings into integers.

Other file servers, as the upasfs(4) mail file server, split the mailboxes
into multiple directory and files. The mailbox itself becomes a directory
under /mail/fs. Each message in the mailbox becomes a numbered directory in the
mailbox directory, and each attachment becomes a numbered directory in the
message directory. Then, each message and attachment directory contains the
files: body, cc, date, digest, disposition, filename, from, header, info,
inreplyto, mimeheader, raw, rawbody, rawheader, replyto, subject.

Each individual file gives a specific information from the email. This makes
the the task of mail readers easy, as they can directly access each information
without having to parse custom formats. But still each file is a unidimensional
byte array that some need individual parsing (eg.: header, date, cc, inreplyto,
etc).

Most software makes use of a hierarchical resource system, like the URL in the
HTTP protocol and a big amount of the protocol designs are just get the content
of some resource, update the content or create a new resource. That maps 
perfectly to the file interface (read, write, create) but it’s impractical for
internet applications as shown earlier.

A new option would be a new file interface supporting structured data.

TODO: talk about Nebula filesystem [4]
https://pdfs.semanticscholar.org/99bc/fbe095b591c696809ca21533bcd1d5502587.pdf


## Introducing Typed Files

From the words of Dennis Ritchie and Ken Thompson:

```
A file contains whatever information the user places on
it, for example symbolic or binary (object) programs. No
particular structuring is expected by the system. Files of
text consist simply of a string of characters, with lines
demarcated by the new-line character. Binary programs are
sequences of words as they will appear in core memory
when the program starts executing. A few user programs
manipulate files with more structure: the assembler
generates and the loader expects an object file in a
particular format. However, the structure of files is
controlled by the programs which use them, not by the system.

From: The UNIX TimeSharing System - Dennis Ritchie and Ken Thompson
```

```
In the UNIX system, a file is a (one-dimensional) array of bytes.
No other structure of files is implied by the system. Files are attached
anywhere (and possibly multiply) onto a hierarchy of directories.
Directories are simply files that users cannot write.

From: Unix Implementation - Ken Thompson
```

A typed file is a file that has an internal structure defined by a type
signature. That way, the Unix concept of a file is just a file of type
“byte array” or []byte.

Below are some examples of file types (syntax similar to Go):

```go
[]byte      // unix file, you could store anything byte representable.

[4]uint8    // For storing an IP address

string      // for storing UTF-8 string

// For storing a structured data
struct {
    name string
    age int
    birthday datetime
    isdeleted bool
}

// For storing an array of structured data
[]struct{
    name string
    age int
    birthday datetime
    isdeleted bool
}
```

When structured, it’s not wrong to say that the file idea starts to confuse
with a filesystem or a database and this similarity will be more evident in the
subsequent sections.

Then, if a file has the following type signature:

```go
struct {
    title string
    year int
    author string
    body string
}
```

And if it has the following content (using JSON syntax for convenience):

```json
{
    "title": "From the Earth to the Moon",
    "year": 1875,
    "author": "Jules Verne",
    "body": "... very long string ..."
}
```

Then, the typed file interface allows for getting (and setting) each of those
fields separately without requiring to read the full file.
The Operating System interface must have a way to answer questions like below
(given that F is an open File Handle):

1. From F, give me the type signature;
2. From F, give me the field called "title";
3. From F, give me the fields "title" and "author";
4. From F, set field "title" with value "Journey To The Centre Of The Earth";
5. From F, give me the byte array representation of the file;

_Requirement 1_ make it clear that the implementation must store the type
signature in the file system as well (or have a way to infer it from the stored
data).

_Requirement 2_ is simple, but what if the field content is bigger than the available memory? What if the field contains another structure?

_Requirement 3 and 4_, makes it clear that some mechanism for getting and
setting multiple fields at once is required;
_Requirement 5_ says that even for structured files, UNIX commands like cat,
grep and so on, must work as expected;

In order to deal with huge field contents, a kernel implementation must have a
way to read and write the field contents in blocks, much like the ordinary read
and write of Unix.

There’s a way to implement that without the burden of having separate complex
get and set syscalls, like adding offset and count to them. This could be
achieved by asking for a new file descriptor for the field and then using the
ordinary read and write syscalls on it.
This idea opens another interesting design idea that can be explored by an
hypothetical new operating system, tha's the ability to open a file field
directly. Take a look in the hypothetical shell session below:

```
$ ls
config src Makefile
$ fs/type ./config
struct {
    name string
    port uint16
    attrs []struct{
        name string
        value string
    }
}
$ cat ./config
{
    "host": "127.0.0.1",
    "port": 6666,
    "attrs": [
        {
            "name": "author",
            "value": "unknown"
        }
    ]
}
$ cat ./config.host
127.0.0.1
```

In the example above, “config” is a typed file and its content isn’t a JSON but
cat (using open, read, close syscalls) applied on the file name reads the byte
representation of the file structure. Opening “./config.host” is the same as
opening the “./config” and asking a file handle for the host field.
The example above adds a different semantic for paths where dots (.) references
field attributes of the file, just as an example for a file system design, that
adds the possibility to reach a specific content (a field) inside the file
directly.

This other example below is a design strategy to achieve similar file content
addressing:

```
$ ls
config src Makefile
$ fs/type ./config
struct {
    name string
    port uint16
    attrs []struct{
        name string
        value string
    }
}
$ fs/mount ./config /n/app/config
$ ls /n/app/config
host port attrs dbconfig
$ cat /n/app/config/host
127.0.0.1
$ cat /n/app/config/port
6666
$ ls /n/app/config/attrs
0 1 2 3 4 5 6
$ ls /n/app/config/attrs/0
name value
$ cat /n/app/config/attrs/0/*
author unknown
```

In the example above, the mount syscall is used to create a file tree similar
to the workaround presented in the last section but using only one real typed
file. The mount syscall will get a file descriptor for the config file using
the ordinary open system call, then the kernel will ask for the file type
signature (maybe using the stat syscall) and then mount a filesystem at the
destination directory. After that, every open syscall happening on the fields
(files on the target directory /n/app/) inside the mounted directory will
operate on the opened file descriptor for the original “config” file (as they
have the same file interface).

## Fundamental Theorem

On most operating systems, a filesystem is an interface to keep track of files
and directories on a disk, partition or even a file. As on most systems, a disk,
a partition and a file are all one-dimensional byte addressable structures,
then the filesystems drivers maintain a searchable data structure that
represents the hierarchical data on them. Each filesystem driver has different
ways to achieve its goals for performance, integrity, security and so on.

The UNIX guys did an important step by representing a disk device as a special
file in the file system (/dev/sda1) and this allows for a lot of interesting
features, as for example the ability to backup a disk by copying its raw
content into a file in another filesystem
(`dd if=/dev/sda1 of=/backups/sda1.backup`) or create a file system inside a
file (`mkfs.ext3 ./file`) and mount this file in the syntetic file system
(`mount -o loop -t ext3 ./file /n`).

This conveniences can be extended even further if we introduce other kinds of
file types other than byte-arrays. A disk with an ext3 filesystem could be
thought as a structured typed file where its directories are fields and the
files are all of type byte array (on existing filesystems like NTFS, ext3, ext4
and so on).

This idea leads to the following statements:

- Mount is the action of exploding the file system structure members of a file
into a path.
- Unmount is the action of imploding (join the members) of a path into a
structure.
- _Traditional syntetic file systems are specialized use cases of a typed
syntetic file systems where all members are unidimensional byte arrays._
- _Mounting a unidimensional disk is the same as mounting a byte-array file._
- All field types are byte addressable.

When you use a mounted filesystem, the system calls (open, read, etc) operate
on a file descriptor, that’s a generic handle that works across any kind of
filesystem. In the lower level, the kernel calls the filesystem API to walk
through the data structures on it to fetch or overwrite data.
Typed files stores their data on a binary representation of the hierarchical
type as well, and much like the filesystems, several implementations apply
depending on the goals.

Have a look in the example below:

```
$ touch ./disk.ext3
$ fs/type ./disk.ext3
[]byte
$ fs/mkfs.ext3 ./disk.ext3
$ fs/type ./disk.ext3
[]byte
$ fs/mount ./disk.ext3 /n/disk
$ mkdir /n/disk/dir
$ echo "hello" > /n/disk/dir/msg
$ unmount /n/disk
$ fs/type ./disk.ext3
struct {
    dir struct {
        msg string
    }
}
$ cat ./disk.ext3
{
    "dir": {
        "msg": "hello"
    }
}
```

As we have hierarchical file systems, having structured files makes it easy to
map from one to another. The reverse could work also, like unmounting
(imploding) a directory into a file. Eg.:

```
$ fs/mount -l
# mnt dev engine
/ /dev/sda1 ext4
/n /dev/sda2 otherfs
$ mkdir /n/app
$ echo "localhost" > /n/app/host
$ echo "6666" > /n/app/port
$ ls /n
app
$ ls /n/app
host port
$ fs/unmount /n/app ./app.fs
$ ls /n
$ ls
app.fs
$ fs/type app.fs
struct {
       host string
       port string
}
$ cat app.fs | grep "localhost"
       "host": "localhost",
$
```

In the example above, the created file “app.fs” is logical. It’s just an entry
in the union mount table of the current namespace. Invoking the read syscall in
an open file handle of “app.fs” will return the byte representation of the
directory `/n/app` from the otherfs filesystem engine backed by `/dev/sda2`.
The kernel will traverse the directories there as if it were a typed file.


# File System API

Below are several examples of typed hierarchical file system API. The first
examples explore the high-level API in programming languages and later about
the OS syscall API.

## Programming language examples

Below is a simple system call proposal for a hypothetical operating system:

The programming languages that supports typed structures can make use of its
in memory structs to hold file system data. Eg.:

```C
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mount.h>

typedef struct {
    const char  *host;
    uint16_t    port;
    struct {
        const char  *user;
        const char  *group;
    } credentials;
} settings;

int createat(int fd, const char *name, const char *type) {
    return openat(fd, name, type, O_CREAT|O_WRONLY|O_TRUNC, 0644);
}

int create(const char *name, const char *type) {
    int  dot;

    dot = open(".", "struct", O_WRONLY, 0);
    if (dot < 0) {
        return -1;
    }

    return createat(dot, name, type);
}

void main() {
    int       fd, field;
    settings  s;

    s.host = "localhost";
    s.port = 1337;
    s.credentials.user = "i4k";
    s.credentials.group = "adm";

    /* creates a new member field in the current directory struct */
    fd = create("settings", "struct")
    if (fd < 0) {
        fatal("create file");
    }

     /* error handling ommited for clarity */
    createat(fd, "host", "string");
    createat(fd, "port", "uint16");

    field = createat(fd, "credentials", "struct");
    if (field < 0) {
        fatal("create credentials schema");
    }

    createat(field, "user", "string");
    createat(field, "group", "string");

    if (write(fd, &s, sizeof(s)) < 0) {
        fatal("write data");
    }

    return;
}
```

In the example above, `fd` and `field` are both file descriptors (remember that
a file is just a field of a structure). The open(2) syscall now has the
following signature:

```
int open(const char *name, const char *type, int flags, mode_t mode);
```

The open(2) syscall try to open `name` path using `type` (the flags and mode
has the same semantics as POSIX). If the internal type of `name` don't match
the one supplied then an error should occur, but with the exception of
byte-array. Any file can be opened as byte array.

If a file is opened as byte-array but it has other internal type, then writing
to the file destroys the data structure. An operating system implementation can
choose to only allow opening for read in such cases.

The openat(2) syscall can be used to open a file descriptor for a struct member
of another file descriptor. Its signature is:

```C
int openat(int fd, const char *field, const char *type, int flags, mode_t mode);
```

It has the same semantics as open but opening a field on a struct file
descriptor. The `field` cannot contains slashes as it should be the field name.

TODO: how write works? how data gets serialized to and from kernel?

## Example of novel command line UNIX-like tools

The design proposed here enables interesting tools and simplify several
programs. As an example, there's no need for configuration file formats and
also binary formats are not needed for several use cases.

For example, a hypotethical operating system can chose to store the executable binaries in a struct file instead of a byte-array with a custom binary format
(ELF, PE, etc).
This allows for an easy design of the assembler, compiler, linker and loader,
while also making it easy to inspect the binary file. Eg.:

```
$ file /bin/ls
exec
$ fs/type /bin/ls
struct {
        type string
        entry uintptr
        segments [3]struct{
                vaddr uintptr
                name string
                data []byte
        }
        sections [100]struct{
                name string
                segment int
                value []byte
        }
}
$ # See next section for details on `get` and `set` programs.
$ get /bin/ls type
exec
$ get /bin/ls entry
1000000
$ get /bin/ls segments[0].name
.text
$ mount /bin/ls /n/ls
$ cd /n/ls
$ ls
type entry segments sections
$ cat type
exec
$ cat entry
1000000
$ ls segments
0 1 2
$ ls segments/0
vaddr name data
$ cat segments/0/name
.text
$ disas segments/0/data
mov 0, %rbx
sub 0x10, %rsp
lea 8(%rsp), %rdi
call 0x2321000
...
```

This design avoids the complexity of executable file formats.

### get and set

The `get` and `set` program can be used to get and set fields in structured
files very easily from the shell.
Let's say you want a structured file for app settings:

```sh
$ pwd
/n/app
$ ls
$ set settings name=app host=localhost port=8080
$ cat settings
name = "app"
host = "localhost"
port = 8080
$
```

Then you can update them easily as well:

```sh
$ set settings host=www.madlambda.io port=8081
$ cat settings
name = "app"
host = "www.madlambda.io"
port = 8081
```

The example code below shows how to make this work for string fields only:

```c
int main(int argc, char **argv)
{
    int root, fd, nargs;
    char *fname, *field, *value, **p;

    if (argc < 4) {
        usage("set file name value\n");
        return 1;
    }

    nargs = argc - 2;
    fname = argv[1];
    p = &argv[2];

    /* open or create a struct if not existant */
    root = open(fname, "struct", O_WRONLY|O_CREAT, 0664);
    if (root < 0) {
        if (errno == ETYPE) {
            fprintf(stderr, "file %s is not a struct\n", argv[1]);
            return 1;
        }

        fatal("failed to open");
    }

    for (int i = 0; i < nargs; i++) {
        field = p[i];
        value = strchr(field, '=');
        if (value == NULL) {
            continue;
        }

        *value = '\0';
        value++;

        fd = openat(root, field, "string", O_WRONLY|O_CREAT, 0644);
        if (fd < 0) {
            fatal("create field"); /* ENOSPC, ETYPE, EPERM, etc */
        }

        if (write(fd, value, strlen(value)) < 0) {
            fatal("write field");
        }
    }

    return 0;
}
```

# Network File Systems

A typed file system can also benefit network file systems by providing means of
requesting specific fields of a structure instead of downloading the full file.
Executing a binary that locates in a remote file system (eg.: /n/office/bin)
can be faster because the loader can fetch just the fields required to load
segments in memory (debugging sections and all non-reachable sections can be
skipped).

In the case of syncing files with the server, just the field members that
changed need to be sent instead of the complete file. By using traditional
file systems, if just 1 bit of the file changes, it makes it required for the
sync of the full file in most of the cases.


1. Jeff Dean: http://static.googleusercontent.com/media/research.google.com/en/us/people/jeff/stanford-295-talk.pdf
2. http://9p.cat-v.org/
3. https://en.wikipedia.org/wiki/Network_File_System
4. C. Dharap, R. Balay, M. Bowman, Type structured file system, December 1993.
