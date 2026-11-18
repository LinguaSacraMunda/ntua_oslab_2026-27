/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * < Your name here >
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "asm-generic/bug.h"
#include "asm-generic/errno-base.h"
#include "asm-generic/fcntl.h"
#include "linux/gfp_types.h"
#include "linux/semaphore.h"
#include "linux/timekeeping.h"
#include "linux/wait.h"
#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

/*
 * Global data
 */
struct cdev lunix_chrdev_cdev;

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
/*
 * Declare a prototype so we can define the "unused" attribute and keep
 * the compiler happy. This function is not yet used, because this helpcode
 * is a stub.
 */
static int __attribute__((unused)) lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *);
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor = state->sensor;
	
	WARN_ON ( !(sensor = state->sensor));
	/* ? */

    // Check last sensor update timestamp
    uint32_t stimestamp = sensor->msr_data[state->type]->last_update;
    uint32_t ttimestamp = state->buf_timestamp;

    // If new data has been received since last
    // textual update, then refresh state
    return (ttimestamp < stimestamp);


	/* The following return is bogus, just for the stub to compile */
    //return 0; /* ? */
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	//struct lunix_sensor_struct __attribute__((unused)) *sensor = state->sensor;
	struct lunix_sensor_struct *sensor = state->sensor;
    struct lunix_msr_data_struct *msr = sensor->msr_data[state->type];
    unsigned long flags = 0;
	
	debug("entering\n");

    WARN_ON(msr->magic != LUNIX_MSR_MAGIC);

	/*
	 * Grab the raw data quickly, hold the
	 * spinlock for as little as possible.
	 */
	/* ? */

    // __func__ is called from open, which is interruptible,
    // so save the irq value and spin
    spin_lock_irqsave(&sensor->lock, flags);

	/* Why use spinlocks? See LDD3, p. 119 */

	/*
	 * Any new data available?
	 */
	/* ? */

    if (!lunix_chrdev_state_needs_refresh(state)) {
        debug("%s: no new data; returning EAGAIN\n", __func__);
        return -EAGAIN;
    }

    // There is new data to parse
    uint32_t values = msr->values[0];

    spin_unlock_irqrestore(&sensor->lock, flags);

	/*
	 * Now we can take our time to format them,
	 * holding only the private state semaphore
	 */

	/* ? */

    // Hold the lock while processing data
    // Since this function is called within a locked
    // code segment, we continue as normal
    
    long cdata;
    switch (state->type) {
    case BATT:
            cdata = lookup_voltage[values];
            break;
    case TEMP:
            cdata = lookup_temperature[values];
            break;
    case LIGHT:
            cdata = lookup_light[values];
            break;
    default:
            debug("%s: invalid sensor type %u\n", __func__, state->type);
            return -ENODEV;
            break;
    }

    // Parse cooked data +/-xx.yyy
    uint8_t sgn = cdata >= 0;
    if (!sgn)
        cdata = -cdata;

    uint32_t upper = cdata / 1000;
    uint32_t lower = cdata % 1000;

    state->buf_lim = snprintf(
            state->buf_data,
            LUNIX_CHRDEV_BUFSZ,
            "%c%u.%03u",
            (sgn ? '+' : '-'), upper, lower);

    if (state->buf_lim <= 0) {
        debug("%s: data parsing: snprintf failed to write\n", __func__);
        return -EIO;
    }

    state->buf_timestamp = ktime_get_real_seconds();
	debug("leaving\n");
	return 0;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	/* Declarations */
	/* ? */
	int ret;

    unsigned int minor;
    unsigned int sensor_id;
    unsigned int measur_id;

    struct lunix_chrdev_state_struct *state;

	debug("entering\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	 */

    minor = iminor(inode);
    sensor_id = minor / LUNIX_MSRMAX_CNT;
    measur_id = minor % LUNIX_MSRMAX_CNT;

    // Check if sensor is within range
    if (sensor_id >= LUNIX_SENSOR_CNT) {
        debug("%s: invalid sensor_id %d\n", __func__, sensor_id);
        ret = -ENODEV;
        goto out;
    }


	/* Allocate a new Lunix character device private state structure */
	/* ? */

    // kzalloc -> allocate size memory and initialise it with 0
    // GFP_KERNEL -> we execute on behalf of a userspace procc, so 
    //               we must be able to sleep while memory is being allocated.
    //               Also, mem is allocated from standard kernel mem zone (ZONE_NORMAL),
    //               and the process may repeat till success
    state = kzalloc(sizeof(struct lunix_chrdev_state_struct), GFP_KERNEL);
    if (!state) {
        printk(KERN_ERR "Failed to allocate memory for Lunix sensors\n");
		goto out;
    }

    // Initialise spinlock and queue; allocate msr_data 
    ret = lunix_sensor_init(state->sensor);
    if (!ret) {
        debug("%s: sensor init failed with error %d\n", __func__, ret);
        goto out_with_state_alloc;
    }

    // Assign sensor id
    state->id = sensor_id;

    // Assign measurement type
    if (measur_id < N_LUNIX_MSR)
        state->type = (enum lunix_msr_enum)measur_id;
    else
        state->type = N_LUNIX_MSR;

    // Set flags: processed data by default
    state->mflags |= filp->f_flags & O_NONBLOCK ? LUNIX_MFLAG_NBLCK : LUNIX_MFLAG_BLOCK;
    state->mflags |= LUNIX_MFLAG_DCOOK;


    // Initialise semaphore as mutex
    // TODO check lock type and sem value
    sema_init(&state->lock, 1);

    // Initialise textual buffer
    // We could also use ktime_get_real_seconds()
    // for the timestamp, however on first read any 
    // clock value less that the current one is valid
    state->buf_lim = 0;
    state->buf_timestamp = 0;
    state->buf_data[state->buf_lim] = '\0';

    filp->private_data = state;

    // Successfully opened cdev
    return 0;

out_with_state_alloc:
    kfree(state);
out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	/* ? */
    struct lunix_chrdev_state_struct *state = filp->private_data;

    // Free sensor->msr_data
    lunix_sensor_destroy(state->sensor);

    // Free allocated lunix_chrdev_state_struct (open)
    kfree(state);
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{
	ssize_t ret;

	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	state = filp->private_data;
	WARN_ON(!state);

	sensor = state->sensor;
	WARN_ON(!sensor);

	/* Lock? */
    // Read must be interruptible
    // If interrupted, down_inter returns a non-zero value
    // and the called does not hold the semaphore
    if (down_interruptible(&state->lock)) {
        return -ERESTARTSYS;
    }
	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */
	if (*f_pos == 0) {
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
			/* ? */
			/* The process needs to sleep */
			/* See LDD3, page 153 for a hint */

            // Release lock before sleeping
            up(&state->lock);

            // Devices opened as non-blocking; return immediately
            if (filp->f_flags & O_NONBLOCK)
                return -EAGAIN;

            // Sleep until new data is received
            // TODO check wait condition
            // TODO check errno
            if (wait_event_interruptible(sensor->wq, 
                        lunix_chrdev_state_update(state) != -EAGAIN))
                return -ERESTARTSYS;

            if (down_interruptible(&state->lock))
                return -ERESTARTSYS;

		}
	}

	/* End of file */
	/* ? */
	
	/* Determine the number of cached bytes to copy to userspace */
	/* ? */

	/* Auto-rewind on EOF mode? */
	/* ? */

	/*
	 * The next two lines  are just meant to suppress a compiler warning
	 * for the "unused" out: label, and for the uninitialized "ret" value.
	 * It's true, this helpcode is a stub, and doesn't use them properly.
	 * Remove them when you've started working on this code.
	 */
	ret = -ENODEV;
	goto out;
out:
	/* Unlock? */
    up(&state->lock);
	return ret;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static struct file_operations lunix_chrdev_fops = 
{
	.owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

int lunix_chrdev_init(void)
{
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	debug("initializing character device\n");
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);
	lunix_chrdev_cdev.owner = THIS_MODULE;
	
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	/* ? */
	/* register_chrdev_region? */

    // Register a region of 16 * 8 devices
    ret = register_chrdev_region(dev_no, lunix_minor_cnt, "lunix_sensor");

	/* Since this code is a stub, exit early */
	//return 0;

	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}
	/* ? */
	/* cdev_add? */

    // lunix_chrdev_cdev is initialised above
    // Tell the Kernel to activate the sensor devices
    ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt);

	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("leaving\n");
}
