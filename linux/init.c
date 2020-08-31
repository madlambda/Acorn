/*
 * Copyright (C) Madlambda Authors.
 */


#include <acorn.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/kmod.h>

#include "ioctl.h"
#include "fs.h"


MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Madlambda Authors");
MODULE_DESCRIPTION("Acorn File System");
MODULE_VERSION("0.01");

#define MINOR_NR    0
#define MINOR_COUNT 1


static long ioctl(struct file *f, unsigned int cmd, unsigned long arg);


static struct file_operations filehandlers =
{
    .owner          = THIS_MODULE,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl          = ioctl
#else
    .unlocked_ioctl = ioctl
#endif
};

static int           major;
static dev_t         dev;
static struct cdev   chardev;
static struct class  *cl;


static long
ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    String  devname;

    debug("ioctl called\n");

    if (cmd == IOCTL_INIT_FS) {
        copy_from_user(&devname, (String *) arg, sizeof(String));

        return fsinit(&devname);

    } else {
        return -ENOTTY;
    }

    return 0;
}


static int __init
loadmod(void)
{
    int            ret;
    struct device  *devret;

    debug("Initializing...\n");

    ret = alloc_chrdev_region(&dev, MINOR_NR, MINOR_COUNT, "acorn");
    if (ret < 0) {
        return ret;
    }

    major = MAJOR(dev);

    debug("Major: %d\n", major);

    cdev_init(&chardev, &filehandlers);

    ret = cdev_add(&chardev, dev, MINOR_COUNT);
    if (ret < 0) {
        return ret;
    }

    cl = class_create(THIS_MODULE, "acorn");

    if (IS_ERR(cl)) {
        cdev_del(&chardev);
        unregister_chrdev_region(dev, MINOR_COUNT);
        return PTR_ERR(cl);
    }

    devret = device_create(cl, NULL, dev, NULL, "acorn");

    if (IS_ERR(devret)) {
        class_destroy(cl);
        cdev_del(&chardev);
        unregister_chrdev_region(dev, MINOR_COUNT);
        return PTR_ERR(devret);
    }

    debug("success.\n");

    return 0;
}


static void __exit
unloadmod(void) {
    debug("unloading...");
    device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&chardev);
	unregister_chrdev_region(dev, MINOR_COUNT);
}


module_init(loadmod);
module_exit(unloadmod);
