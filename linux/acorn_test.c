/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ioctl.h"


static int major = ACORN_MODULE_MAJOR_NR;


int main()
{
    int          fd, configfd, rc;
    AcornCreate  create;
    AcornWrite   write;

    static char    *devname   = "/dev/acorn";
    static String  blkdevname = str("/dev/loop0");
    static String  configname = str("config");
    static String  namename   = str("name");
    static String  hostname   = str("host");
    static String  portname   = str("port");
    static String  debugname  = str("debug");
    static String  xpto       = str("XPTO");
    static String  localhost  = str("localhost");

    fd = open(devname, O_RDONLY);
    if (slow(fd < 0)) {
        perror("failed to open file");
        return 1;
    }

    rc = ioctl(fd, ACORN_INIT_FS, &blkdevname);
    if (slow(rc < 0)) {
        perror("failed to initialize the file system");
        return 1;
    }

    /*
     * Acorn Userspace Program
     *
     * int main() {
     *     int  fd;
     *
     *     fd = create(newcstring("./config"), TYPE_STRUCT);
     *     if (fd < 0) abort("failed to open file");
     *
     *     Will lead to Acorn kernel code below:
     *     ...
     */
    bzero(&create, sizeof(create));
    create.path = abspath(configname);
    create.mode = 0644;
    create.type = TYPE_STRUCT;
    rc = ioctl(fd, ACORN_CREATE, &create);
    if (slow(rc < 0 || create.fd < 0)) {
        perror("failed to create 'config");
        return 1;
    }

    /*
     * create.fd is set to open file descriptor for "config" and
     * it can be used to create other files/attributes on it.
    */
    configfd = create.fd; /* store the parent fd */

    /*
     * Userspace
     * 
     *     int     namefd;
     *     String  defaultName = str("XPTO");
     * 
     *     namefd = createat(fd, newcstring("name"), TYPE_STRING);
     *     if (namefd < 0) abort("failed to create 'config.name");
     * 
     *     write(namefd, TYPE_STRING, &defaultName);
     */
    create.path = newcstring("name");
    create.mode = 0644;
    create.type = TYPE_STRING;
    rc = ioctl(fd, ACORN_CREATE, &create);
    if (slow(rc < 0 || create.fd < 0)) {
        perror("failed to create 'config.name'");
        return 1;
    }

    bzero(&write, sizeof(write));

    write.fd = create.fd;
    write.type = TYPE_STRING;
    write.data = &xpto;
    rc = ioctl(fd, ACORN_WRITE, &write);
    if (slow(rc < 0)) {
        perror("failed to write to config.name");
        return 1;
    }

    close(create.fd);

    /*
     * Userspace
     * 
     *     int     hostfd;
     *     String  defaultHost = str("localhost");
     * 
     *     hostfd = createat(fd, newcstring("host"), TYPE_STRING);
     *     if (hostfd < 0) abort("failed to create 'config.host");
     * 
     *     write(hostfd, TYPE_STRING, &defaultHost);
     */

    create.fd = configfd;
    create.path = newcstring("host");
    create.mode = 0644;
    create.type = TYPE_STRING;
    rc = ioctl(fd, ACORN_CREATE, &create);
    if (slow(rc < 0 || create.fd < 0)) {
        perror("failed to create 'config.host'");
        return 1;
    }

    write.fd = create.fd;
    write.type = TYPE_STRING;
    write.data = &localhost;
    rc = ioctl(fd, ACORN_WRITE, &write);
    if (slow(rc < 0)) {
        perror("failed to write to config.host");
        return 1;
    }

    close(create.fd);

    /*
     * Userspace
     * 
     *     int  portfd;
     * 
     *     portfd = createat(fd, newcstring("port"), TYPE_UINT16);
     *     if (portfd < 0) abort("failed to create 'config.port");
     * 
     *     write(portfd, TYPE_UINT16, 8080);
     */
    create.fd = configfd;
    create.path = newcstring("port");
    create.mode = 0644;
    create.type = TYPE_UINT16;
    rc = ioctl(fd, ACORN_CREATE, &create);
    if (slow(rc < 0 || create.fd < 0)) {
        perror("failed to create 'config.host'");
        return 1;
    }

    write.fd = create.fd;
    write.type = TYPE_UINT16;

    u16  port = 8080;

    write.data = &port;
    rc = ioctl(fd, ACORN_WRITE, &write);
    if (slow(rc < 0)) {
        perror("failed to write to config.port");
        return 1;
    }

    close(create.fd);

    /*
     * Userspace:
     * 
     *     int  rc;
     *
     *     rc = mount(fd, "/tmp");
     *     if (rc < 0) abort("failed to mount 'config' at /tmp");
     * 
     *     system("cat /tmp/config/host"); // prints "localhost"
     * 
     *     unmount("/tmp/config");
     * 
     *     return 0;
     */

    /* mount tree is maintained by Acorn */
    rc = mount(fd, "/tmp");
    if (slow(rc < 0)) {
        perror("mount failed");
        return 1;
    }

    /*
     * "cat" open(2) syscall will traverse the Acorn union mount.
     * /tmp/config is a mount for a block device (disk) then Acorn will issue
     * Linux module calls to traverse the typed file system if needed.
     */
    rc = exec("cat", "/tmp/config/host", NULL);
    if (rc < 0) {
        perror("exec failed");
        return 1;
    }

    close(fd);

    return 0;
}
