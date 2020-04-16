# The Fundamental Theorem of Hierarchical File Systems

## Introduction

The concept of a file is the same (or very close) on most operating systems of
today. The content of a file is a one-dimensional byte array where the meaning
of its content comes from the reader. The basic file operations are: create,
open, read, write and close. That simplicity has a huge impact on the success
of Multics and Unix, and it still benefits today's software. On Unix based
operating systems, the input and output of programs are (special) files. This
way, the output of one program (the standard output file or stdout) could be
piped into the input (the standard input file or stdin) of another to solve
complex tasks by just using file manipulation tools. For this to work, a
special kind of file, called pipe, and a convention (eg.: read from stdin,
write to stdout, config from arguments, seeks are not allowed or ignored, and
so on) must be in place:

If the file is on disk, then the time to sequentially read 1 MB of its content
is around 20ms (or 150 us from an SSD) and if the file is a pipe, then the same
read could spend 250 us. But if the file is remote then the time goes to a few
seconds.[1] As the file content does not have an internal structure, a program
that needs to retrieve some specific information inside it needs to read all
its content or at least some few blocks until it could seek to the information
if it has a header section defined. Most files that need to be consumed only by
computer programs have a file format to ease the process of traversing its
internal data. One way to work around that problem is to split the resource
information into several files inside a folder. That way, specific fields could
be directly addressed but the file system is hard to maintain. As an example,
think about a user’s detail information modeled on several files:

```
$ ls /users/i4k
firstname surname birthday profession country state address zip
```

If the resource has hundreds of fields, as much of the real world applications
have to deal with JSON APIs that reply with hundreds of them, it would be a
nightmare to manage.
Another problem is that file operation being stateful, the kernel is the
Centralized State Store when managing files local on a single machine, but when
the file system is remote such thing doesn’t exist (at least not on 9P). The
NFS is stateless but doesn’t support custom file servers (applications). Being
stateful means it’s hard to replicate data, load balance, scale file servers up
and down and so on.
Because of that, the file abstraction (when exposed through a network protocol
like 9P[2] or NFS[3]) is not suited for internet applications.

Modern software that communicates on the internet commonly does not share files
but uses general protocols (HTTP/2/3, GRPC and so on) that support a way to
express the semantics of the communication and scales well. Eg.: When using
HTTP, the client software could express the intention of requesting the name
and email of the user with id equals 10 using the GET method (or verb):

```
GET /users/10?fields=name,email
```

That way, even if the server store thousand users and even if a user is
comprised of thousand fields, the server could comprehend the client’s
intention and reply with just name and email, saving a lot of bandwidth and
time.

The file interface is heavy used in the design of most operating systems when a
specialized syscall is not required (procfs, cgroups, and so on) but
nonexistent in the design of distributed software.

Most software makes use of a hierarchical resource system, like the URL in the
HTTP protocol and a big amount of the protocol designs are just get the content
of some resource, update the content or create a new resource. That’s just the
file interface (read, write, create) but it’s useless for internet applications
as shown earlier.

The only option would be a new file interface supporting a mechanism of
querying his internal data, in other words, the read syscall should be used to
transport only the data the client demands.

TODO: talk about Nebula filesystem
https://pdfs.semanticscholar.org/99bc/fbe095b591c696809ca21533bcd1d5502587.pdf


## Typed Files

A typed file is a file that has an internal structure defined by a type
signature. That way, the Unix concept of a file is just a file of type
“byte array”.

Below are some examples of file types (syntax close to Go):

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
(given that F is an open File Descriptor):

1. From F, give me the type signature;
2. From F, give me the field called “title”;
3. From F, give me the fields “title” and “author”;
4. From F, set field “title” with value “Journey To The Centre Of The Earth”;
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
This idea opens another interesting design, the ability to open a file field
directly. Take a look in the hypothetical shell session below:

```
$ ls
config src Makefile
$ fs/type -schema ./config
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
opening the “./config” and asking a file descriptor for the host field.
The example above adds a different semantic for paths where dots (.) references
field attributes of the file, just as an example for a file system design, that
adds the possibility to reach a specific content (a field) inside the file
directly.

This other example below is a design strategy to achieve similar file content
addressing:

```
$ ls
config src Makefile
$ fs/type -schema ./config
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

The UNIX guys did an important step by representing a disk device as a file in
the file system (/dev/sda1) and this allows for a lot of interesting features,
as for example the ability to backup a disk by copying its raw content into
a file in another filesystem (`dd if=/dev/sda1 of=/backups/sda1.backup`) or
create a file system inside a file (`mkfs.ext3 ./file`) and mount this file in
the syntetic file system (`mount -o loop -t ext3 ./file /n`).

This conveniences can be extended even further if we introduce other kinds of
file types other than byte-arrays. A disk with an ext3 filesystem could be
thought as a structured typed file where its directories are fields and the
files are all of type byte array (on existing filesystems like NTFS, ext3, ext4
and so on).

This idea leads to the following statements:

- Mount is the action of exploding the file system structure members of a file
into a path.
- Unmount is the action of imploding (join the members) of a path into an
structure.
- _Traditional syntetic file systems are an specialized use case of typed
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
$ fs/type -schema ./disk.ext3
[]byte
$ fs/mkfs.ext3 ./disk.ext3
$ fs/type -schema ./disk.ext3
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
an open file descriptor of “app.fs” will return the byte representation of the
directory `/n/app` from the otherfs filesystem engine backed by `/dev/sda2`.
The kernel will traverse the directories there as if it were a typed file.


# File System API

Below are several examples of typed hierarchical file system API. The first
examples explore the high-level API in programming languages and later about
the OS syscall API.

## Programming language examples

Below is a simple system call proposal for a hypothetical (new) operating
system:

The programming languages that supports typed structures can make use of its
in memory structs to hold file system data. Eg.:

```C
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
        perror("create file");
    }

     /* error handling ommited for clarity */
    createat(fd, "host", "string");
    createat(fd, "port", "uint16");

    field = createat(fd, "credentials", "struct");
    if (field < 0) {
        perror("create credentials schema");
    }

    createat(field, "user", "string");
    createat(field, "group", "string");

    if (write(fd, &s, sizeof(s)) < 0) {
        perror("write data");
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
programs. Eg.:

### set

The `set` program can be used to set fields in structured files very easily.
Let's say you hold a structured file for app settings:

```sh
$ cat settings
name = "app name"
host = "localhost"
port = 8080
$ 
```

Then you can set the fields with:

```sh
$ set settings host="www.madlambda.io" port=8081
$ cat settings
name = "app name"
host = "www.madlambda.io"
port = 8081
```

The example code below shows how to make this for string fields only:

```C
int main(int argc, char **argv)
{
    int fd, field;

    if (argc < 4) {
        usage("set file name value\n");
    }

    /* open or create a struct if not existant */
    fd = open(argv[1], "struct", O_WRONLY|O_CREAT, 0664);
    if (fd < 0) {
        if (errno == ETYPE) {
            fprintf(stderr, "file %s is not a struct\n", argv[1]);
            return 1;
        }

        perror("failed to open");
        return 1;
    }

    field = openat(fd, argv[2], "string", O_WRONLY|O_CREAT, 0644);
    if (field < 0) {
         perror(field); /* ENOSPC, ETYPE, EPERM, etc */
         return 1;
    }

    if (write(field, argv[3], strlen(argv[3])) < 0) {
        perror(NULL);
        return 1;
    }

    /* need close stuff ? */
    return 0;
}
```

1. Jeff Dean: http://static.googleusercontent.com/media/research.google.com/en/us/people/jeff/stanford-295-talk.pdf
2. http://9p.cat-v.org/
3. https://en.wikipedia.org/wiki/Network_File_System
4. 5. C. Dharap, R. Balay, M. Bowman, Type structured file system, December 1993.
