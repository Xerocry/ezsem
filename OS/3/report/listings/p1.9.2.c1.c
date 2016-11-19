#include <stdio.h>
#include <unistd.h>

#define SLEEP_DURATION 1

int main() {
	printf("Child #1 started, pid %d, ppid %d.\n", getpid(), getppid());
	sleep(SLEEP_DURATION);
	return 0x0;
}
