#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>

#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdbool.h> 
#include <sys/syscall.h>

#define SIGTX 44
#define REGISTER_UAPP _IO('R', 'g')

/* 3. Get timer callback from kernel hrtimer */
// gcc testapp.c -o testapp
// ./testapp
// last=4547. 138209563 @ 2 cpu
// last=4547. 158711040 @ 2 cpu
// last=4547. 178311647 @ 2 cpu
// last=4547. 199371559 @ 2 cpu
// last=4547. 218364386 @ 3 cpu
// last=4547. 238414595 @ 0 cpu
// last=4547. 258538373 @ 2 cpu
// last=4547. 278531367 @ 1 cpu
// last=4547. 298589192 @ 2 cpu
// last=4547. 318608202 @ 3 cpu
// last=4547. 338655596 @ 3 cpu

void signalhandler(int sig) {
	struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    printf("last=%ld. %ld @ %d cpu\n", current_time.tv_sec, current_time.tv_nsec, sched_getcpu());
}

int main() {
	int fd;
	signal(SIGTX, signalhandler);

	printf("PID: %d\n", getpid());

	/* Open the device file */
	fd = open("/dev/userspace_hr_timer", O_RDONLY);
	if(fd < 0) {
		perror("Could not open device file");
		return -1;
	}

	/* Register app to KM */
	if(ioctl(fd, REGISTER_UAPP, NULL)) {
		perror("Error registering app");
		close(fd);
		return -1;
	}


	/* Wait for Signal */
	printf("Wait for signal...\n");
	while(1)
		sleep(1);

	return 0;
}


