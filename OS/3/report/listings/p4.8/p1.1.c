#include <stdio.h>
#include <unistd.h>

int main() {
	printf("Process has been started, pid %d, ppid %d.\n", getpid(), getppid());
	
	// Порождаем процесс-потомок
	if(fork())
		printf("Parent process, pid %d, ppid %d.\n", getpid(), getppid());
	else
		printf("Child process, pid %d, ppid %d.\n", getpid(), getppid());
		

	printf("Process has been finished, pid %d, ppid %d.\n", getpid(), getppid());
	return 0x0;
}
