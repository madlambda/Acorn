/*
 * Copyright (C) Madlambda Authors.
 */


#include <acorn.h>
#include "fs.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/bio.h>


typedef struct {
    struct block_device  *devraw;
} TypedFS;


static TypedFS  fs;


int
fsinit(String *devname)
{
    /*u8                   *mem;
    struct bio           *bio;
    struct page          *pg;
    struct completion    event;*/
    struct block_device  *bdevraw;

    debug("initializing file system: %s\n", devname->start);

    bdevraw = lookup_bdev(devname->start, 0 /* TODO(i4k): enable check */);

    if (IS_ERR(bdevraw)) {
        debug("error opening raw device <%lu>\n", PTR_ERR(bdevraw));
        return -EFAULT;
    }

    if (!bdget(bdevraw->bd_dev)) {
        debug("error bdget()\n");
        return -EFAULT;
    }

    if (blkdev_get(bdevraw, (FMODE_READ | FMODE_WRITE | FMODE_EXCL), &fs))
    {
        debug("error blkdev_get()\n");
        bdput(bdevraw);
        return -EFAULT;
    }

    /*
     * TODO

    pg = alloc_page(GFP_KERNEL);

    bio = bio_alloc(GFP_NOIO, 1);

    bio_add_page(bio, pg, 10, 0);

    bio_set_dev(bio, bdevraw);
    bio->bi_iter.bi_sector = 0;

    init_completion(&event);

    bio_set_op_attrs(bio, REQ_OP_READ | REQ_SYNC, 0);
    submit_bio(bio);

    wait_for_completion(&event);

    mem = page_address(pg);

    mem[0] = 'A';
    mem[1] = 'A';
    mem[2] = 'A';

    bio_set_op_attrs(bio, REQ_OP_WRITE | REQ_SYNC, 0);
    submit_bio(bio);

    bio_put(bio);

    */

    return 0;
}
