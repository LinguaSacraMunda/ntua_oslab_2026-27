#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "test.h"

int main(int argc, char** argv) {
	int fd = open("/dev/lunix0-batt", O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "failed to open lunix0-batt\n");
		return 1;
	}

	char buf[64];
	ssize_t rc = read(fd, buf, 64);
	if (rc < 0) {
		fprintf(stderr, "read failed\n");
		return 1;
	}
	buf[rc] = '\0';

	printf("simple read: %s\n", buf);

	uint8_t mflags = LUNIX_MFLAG_DUNTS;
	if (ioctl(fd, LUNIX_IOC_SET, &mflags) < 0) {
		fprintf(stderr, "ioctl DUNT failed\n");
		return 1;
	}

	rc = read(fd, buf, 64);
	if (rc < 0) {
		fprintf(stderr, "read failed\n");
		return 1;
	}
	buf[rc] = '\0';

	printf("units read: %s\n", buf);

	if (ioctl(fd, LUNIX_IOC_RESET) < 0) {
		fprintf(stderr, "reset failed\n");
		return 1;
	}

	mflags = LUNIX_MFLAG_DTRAW;

	if (ioctl(fd, LUNIX_IOC_SET, &mflags) < 0) {
		fprintf(stderr, "ioctl RAW failed\n");
		return 1;
	}

	rc = read(fd, buf, 64);
	if (rc < 0) {
		fprintf(stderr, "read failed\n");
		return 1;
	}
	buf[rc] = '\0';

	printf("raw read: %s\n", buf);
	close(fd);
	return 0;
}
