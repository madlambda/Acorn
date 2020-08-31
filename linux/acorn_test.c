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
    int   fd, rc;

    static char    *devname = "/dev/acorn";
    static String  blkdevname = str("/dev/loop0");

    fd = open(devname, O_RDONLY);
    if (slow(fd < 0)) {
        perror("failed to open file");
        return 1;
    }

    rc = ioctl(fd, IOCTL_INIT_FS, &blkdevname);
    if (slow(rc < 0)) {
        perror("failed to initialize the file system");
        return 1;
    }

    close(fd);

    return 0;
}
