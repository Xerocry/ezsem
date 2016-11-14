#include <stdio.h>
#include <unistd.h>

#define SLEEP_DURATION 3

int main() {
	printf("Child #3 started, pid %d, ppid %d.\n", getpid(), getppid());
	sleep(SLEEP_DURATION);
	return 0x0;
}
