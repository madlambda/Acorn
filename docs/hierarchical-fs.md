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
seconds. As the file content does not have an internal structure, a program
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

```GET /users/10?fields=name,email```

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

```
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

```
struct {
    title string
    year int
    author string
    body string
}
```

And if it has the following content (using JSON syntax for convenience):

```
{
    "title": "From the Earth to the Moon",
    “year”: 1875,
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
Fundamental Theorem
On most operating systems, a filesystem is an interface to keep track of files
and directories on a disk, partition or even a file. As on most systems, a disk,
a partition and a file are all one-dimensional byte addressable structures,
then the filesystems drivers maintain a binary format of representing the hierarchical data on them. Each filesystem driver has different ways to achieve
its goals for performance, integrity, security and so on.
When you use a mounted filesystem, the system calls (open, read, etc) operate
on a file descriptor, that’s a generic handle that works across any kind of
filesystem. In the lower level, the kernel calls the filesystem API to walk
through the data structures on it to fetch or overwrite data.
Typed files stores their data on a binary representation of the hierarchical
type as well, and much like the filesystems, several implementations apply
depending on the goals.
A disk with an ext3 filesystem could be thought as a structured typed file
where its directories are fields and the files are all of type byte arrays (on
existing filesystems like NTFS, ext3, ext4 and so on).

What does that mean? That we could use the same file interface when working
directly with a raw disk.
Example:

```
$ fs/type -schema ./disk.ext3
[]byte
$ fs/mkfs.ext3 ./disk.ext3
$ fs/type -schema ./disk.ext3
[]byte
$ fs/type -engine ./disk.ext3
default
$ fs/register ext3 ./disk.ext3
$ fs/type -schema ./disk.ext3
struct {}
$ fs/type -engine ./disk.ext3
ext3
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

As we have hierarchical file systems[cite Multics and Unix paper], having
structured files makes it easy to map from one to another. The reverse could
work also, like unmounting a directory into a file. Eg.:

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
$ fs/type -engine app.fs
otherfs
$ cat app.fs | grep "localhost"
       "host": "localhost",
$
```

In the example above, the created file “app.fs” is logical. It’s just an entry
in the union mount table of the current namespace. Invoking the read syscall in
an open file descriptor of “app.fs” will return the byte representation of the
directory `/n/app` from the otherfs filesystem engine backed by `/dev/sda2`.
The kernel will traverse the directories there as if it were a typed file.

The design should deal with the case of nested structures or disallow them. In
case of supporting it (as this expands the range of applications that could
benefit from it), the design should deal with the problem of passing structures
between kernel and processes.
The primitive data types could all be copied, like other system calls, but structures stored in memory have a different layout on each programming
language. Another problem is that structures commonly are not contiguous in
memory, then making it risky for any kind of recursive copy of the data
happening in kernel land (kernel will have to handle cycles in a safe way).

The safest design for this case seems to be the use of a serialization protocol
and then making all SFile related syscalls to use just byte arrays, easily
copied to kernel memory. The downside is performance.


# Rethinking the File System

## Principles

### System Calls

Below is a simple system call proposal for a hypothetical (new) operating
system:

Outdated, initial idea
Programming language examples

Below is an example of a Go program to store an integer:

```go
f, err := os.CreateSFile(“./ctl/offset”, “int”)
if err != nil {
    log.Fatalf(“error: failed to create file”)
}

// err = f.Write([]byte{1, 1, 0, 0}) // error, file is an integer
// err = f.SetData(0.1) // fail

err = f.SetData(1337) // success
```

Below is an example of a Go program that stores a structure:

```go
const fileSpec = `
struct {
    minMemory uint32
    maxMemory uint32
    numCores uint8
}`

f, err := os.CreateSFile("settings", fileSpec)

f.Set("minMemory", 0)
f.Set("maxMemory", 1024*1024)
f.Set(“numCores”, 8)

f.Sync()
```

```go
type KeyVal struct{
    Name string  `nine:”name”`
    Value string `nine:”value”`
}

type ServerCfg struct {
    Attrs []KeyVal `nine:”attrs”`
    Host  string   `nine:”host”`
    Port  unt16    `nine:”port”`
}

cfg := ServerCfg{
    Attrs: []KeyVal{
        {Name: “language”, Value: “go”},
        {Name: “version”, Value: “1.12”},
    },

    Host: “localhost”,
    Port: 6666,
}

f, err := os.CreateSFile(“./server.cfg”, nine.FileSpecFrom(cfg))
handleErr(err)

err = f.SetData(cfg) // serialize object and send kernel message
handleErr(err)
```

Below is the SFile interface in Go:

```
interface File {
    io.Reader   // read(fd, buf, count)
    io.Writer   // write(fd, buf, count)
    io.ReaderAt // seek(fd, kind, pos)
    io.WriterAt // seek(fd, kind, pos)
    io.Closer   // close(fd)
}

interface SFile {
        File

        Set(field Field, value interface{}) error
        SetData(value interface{}) error


        Get(field Field) (interface{}, error)
        GetData(field Field) (interface{}, error)
}
```

1. Jeff Dean: http://static.googleusercontent.com/media/research.google.com/en/us/people/jeff/stanford-295-talk.pdf
2. http://9p.cat-v.org/
3. https://en.wikipedia.org/wiki/Network_File_System
4. 5. C. Dharap, R. Balay, M. Bowman, Type structured file system, December 1993.
