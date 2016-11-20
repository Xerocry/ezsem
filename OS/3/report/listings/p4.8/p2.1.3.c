#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define DELAY 5

int main() {
	printf("Parent pid %d, ppid %d.\n", getpid(), getppid());

	if(!fork()) {
		// Создаем процесс-потомок
		execl("p2.1.1.ch", "p2.1.1.ch", NULL);
	}	

	system("ps xf > p2.1.3.z1.log");
	sleep(DELAY);
	system("ps xf > p2.1.3.z2.log");

	printf("Parent has been finished.\n");

	return 0x0;
}
