#include <stdio.h>
#include <unistd.h>

#define DELAY 3

int main() {
	printf("Child pid %d, ppid %d.\n", getpid(), getppid());
	sleep(DELAY);
	printf("Child has been finished.\n");

	return 0x0;
}
