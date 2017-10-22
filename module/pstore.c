/******************************************************************************
 * File:        pstore.c                                                      *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171011                                                      *
 ******************************************************************************/
#include <linux/blkdev.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/mount.h>
#include <linux/slab.h>
#include "logger.h"
#include "pstore.h"
#define DEVICE_NAME "map"

extern struct kobject *sysfs_dev_block_kobj;

static void print_request(struct request *);

static char *stringcat(const char *s1, const char *s2);
static int create_device(unsigned long capacity);
static void handle_request(struct request_queue *queue);
static void pstore_release(struct gendisk *disk, fmode_t mode);
static int pstore_open(struct block_device *device, fmode_t mode);
static int pstore_ioctl(struct block_device *device, fmode_t mode,
    unsigned int command, unsigned long arg);

static const struct block_device_operations operations = {
  .owner = THIS_MODULE,
  .open = pstore_open,
  .ioctl = pstore_ioctl,
  .release = pstore_release
};

static struct storage {
  int dummy;
} store;

static DEFINE_SPINLOCK(lock);
static DEFINE_MUTEX(mutex);
static int usage_count;
static struct gendisk *disk;
static struct request_queue *requests;
static struct block_device *device;

int pstore_init(unsigned long capacity) {
  char *path;
  dev_t devt;

  /* Get the device (if it exists). */
  path = stringcat("/dev/", DEVICE_NAME);
  devt = name_to_dev_t(path);
  kfree(path);

  /* Create the device if it does not exist. */
  if (!MAJOR(devt) && create_device(capacity)) {
    logger_warn("failed to create device\n");
    return 1;
  }

  /* Restore the device since the previous use. */
  device = bdget(devt);
  if (!device) {
    logger_debug("device exists, but it does not exist.\n");
    return 1;
  }

  if (device->bd_disk) {
    logger_debug("disk capacity: %llu\n", get_capacity(device->bd_disk) * 512);
    logger_debug("queue does%s exist\n", (device->bd_disk->queue ? "" : " NOT"));
  } else {
    logger_debug("disk does not exist\n");
  }

  return 0;
}

int create_device(unsigned long capacity) {
  int major;

  /* Make sure that the stogare capacity is at least 1MiB. */
  capacity *= 1024 * 1024;
  if (!capacity) {
    logger_debug("capacity is %llu\n", capacity);
    return 1;
  }

  /* Register a block device. */
  major = register_blkdev(0, DEVICE_NAME);
  if (!major)
    goto reg_fail;

  /* Allocate a disk. */
  disk = alloc_disk(1);
  if (!disk)
    goto alloc_fail;

  /* Initialize the disk structure. */
  disk->major = major;
  disk->first_minor = 0;
  disk->fops = &operations;
  disk->private_data = &store;
  strcpy(disk->disk_name, DEVICE_NAME);
  set_capacity(disk, capacity / 512);

  /* Add a request queue to the device. */
  requests = blk_init_queue(handle_request, &lock);
  if (!requests)
    goto queue_fail;
  disk->queue = requests;

  /* somthing more here... maybe? */
  usage_count = 0;

  /* Add the device. */
  add_disk(disk);
  return 0;

queue_fail:
  logger_debug("  queue failure\n");
  del_gendisk(disk);
  put_disk(disk);
  vfree(disk);

alloc_fail:
  logger_debug("  alloc failure\n");
  unregister_blkdev(major, DEVICE_NAME);

reg_fail:
  logger_debug("  registration failure\n");
  return 1;
}

void pstore_destroy(void) {
  //blk_cleanup_queue(requests);

  /* Maybe these are exactly what we DON'T want. */
  //del_gendisk(disk);
  //put_disk(disk);
  //unregister_blkdev(major, DEVICE_NAME);
}

void pstore_clear(void) {
}

// void pstore_save(...) {
// }

// void pstore_load(...) {
// }

void handle_request(struct request_queue *queue) {
  struct request *request;

  request = blk_fetch_request(queue);
  while (request) {
    print_request(request);

    //pstore_transfer(request->sector, request->current_nr_sectors,
        //request->buffer, rq_data_dir(request));
    __blk_end_request_all(request, 1);
    request = blk_fetch_request(queue);
  }
}

void print_request(struct request *r) {
  logger_info("_________REQUEST_________\n");
  logger_info("flags:            %llu\n", r->cmd_flags);
  logger_info("type:             %u\n", r->cmd_type);
  logger_info("cpu:              %u\n", r->cpu);
  logger_info("nr_sects:         %llu\n", r->part->nr_sects);
  logger_info("policy:           %d\n", r->part->policy);
  logger_info("stamp:            %lu\n", r->part->stamp);
  logger_info("start_time:       %lu\n", r->start_time);
  logger_info("nr_phys_segments: %llu\n", r->nr_phys_segments);
  logger_info("ioprio:           %u\n", r->ioprio);
  logger_info("tag:              %d\n", r->tag);
  logger_info("errors:           %d\n", r->errors);
  logger_info("deadline:         %lu\n", r->deadline);
  logger_info("timeout:          %u\n", r->timeout);
  logger_info("retries           %d\n\n", r->retries);
}

//void pstore_transfer(sector_t sector, 

int pstore_open(struct block_device *device, fmode_t mode) {
  mutex_lock(&mutex);

  /* Make sure that the data has been initialized. */
  //if (!device->bd_disk || !device->bd_disk->private_data)
    //return -EBUSY;

  usage_count++;
  mutex_unlock(&mutex);
  return 0;
}

static void pstore_release(struct gendisk *disk, fmode_t mode) {
  mutex_lock(&mutex);
  usage_count--;
  mutex_unlock(&mutex);
}

static int pstore_ioctl(struct block_device *device, fmode_t mode,
    unsigned int command, unsigned long arg) {
  int error = 0;

  logger_info("pstore_ioctl\n");
  switch (command) {
  default:
    error = -ENOIOCTLCMD;
    break;
  }

  return 0;
}

char *stringcat(const char *s1, const char *s2) {
  size_t length1;
  size_t length2;
  char *string;

  length1 = strlen(s1);
  length2 = strlen(s2) + 1;

  string = (char *) kmalloc(length1 + length2, GFP_KERNEL);
  memcpy(string, s1, length1);
  memcpy(string + length1, s2, length2);

  return string;
}
