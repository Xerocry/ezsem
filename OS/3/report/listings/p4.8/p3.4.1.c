#include <stdio.h>
#include <unistd.h>

#define COUNT (int)1e6
#define PERIOD (int)1e5

int main() {
	int pid = getpid();

	for(int index = 0; index < COUNT; ++index)
		if(index % PERIOD == 0)
			printf("Pid %d, Index %d.\n", pid, index);

	return 0x0;
}
