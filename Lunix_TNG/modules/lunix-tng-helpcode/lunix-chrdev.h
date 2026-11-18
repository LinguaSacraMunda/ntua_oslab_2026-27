/*
 * lunix-chrdev.h
 *
 * Definition file for the
 * Lunix:TNG character device
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 */

#ifndef _LUNIX_CHRDEV_H
#define _LUNIX_CHRDEV_H

/*
 * Lunix:TNG character device
 */
#define LUNIX_CHRDEV_MAJOR 60   /* Reserved for local / experimental use */
#define LUNIX_CHRDEV_BUFSZ 20   /* Buffer size used to hold textual info */

/* Compile-time parameters */

#ifdef __KERNEL__ 

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "lunix.h"

/*
 * Private state for an open character device node
 */
struct lunix_chrdev_state_struct {
	enum lunix_msr_enum type;
	struct lunix_sensor_struct *sensor;

	/* A buffer used to hold cached textual info */
	int buf_lim;
	unsigned char buf_data[LUNIX_CHRDEV_BUFSZ];
	uint32_t buf_timestamp;

	struct semaphore lock;

	/*
	 * Fixme: Any mode settings? e.g. blocking vs. non-blocking
	 */
    // Bitset of mode flags
    // mflags[0] = blocking/non-blocking
    // mflags[1] = cooked data/raw data (for ioctl)
    uint8_t mflags;


    // Sensor id
    int id;
};

#define LUNIX_MFLAG_BLOCK (1 << 0)
#define LUNIX_MFLAG_NBLCK 0x0
#define LUNIX_MFLAG_DCOOK (1 << 2)
#define LUNIX_MFLAG_DTRAW 0x0

/*
 * Function prototypes
 */
int lunix_chrdev_init(void);
void lunix_chrdev_destroy(void);

#endif /* __KERNEL__ */

#include <linux/ioctl.h>

/*
 * Definition of ioctl commands
 */
#define LUNIX_IOC_MAGIC     LUNIX_CHRDEV_MAJOR
//#define LUNIX_IOC_EXAMPLE _IOR(LUNIX_IOC_MAGIC, 0, void *)

#define LUNIX_IOC_MAXNR 0

#endif /* _LUNIX_H */
