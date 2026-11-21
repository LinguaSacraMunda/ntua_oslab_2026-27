#ifndef _TEST_H
#define _TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define LUNIX_MFLAG_BLOCK (1 << 0)
#define LUNIX_MFLAG_NBLCK 0x0
#define LUNIX_MFLAG_DCOOK (1 << 2)
#define LUNIX_MFLAG_DTRAW (1 << 3)
#define LUNIX_MFLAG_DUNTS (1 << 4)

#define LUNIX_CHRDEV_MAJOR 60   /* Reserved for local / experimental use */
#define LUNIX_CHRDEV_BUFSZ 20   /* Buffer size used to hold textual info */



/*
 * Definition of ioctl commands
 */
#define LUNIX_IOC_MAGIC     LUNIX_CHRDEV_MAJOR
//#define LUNIX_IOC_EXAMPLE _IOR(LUNIX_IOC_MAGIC, 0, void *)

#define LUNIX_IOC_RESET _IO(LUNIX_IOC_MAGIC, 0)
#define LUNIX_IOC_SET _IOW(LUNIX_IOC_MAGIC, 1, int)
#define LUNIX_IOC_GET _IOR(LUNIX_IOC_MAGIC, 2, int)

#define LUNIX_IOC_MAXNR 3

#endif
