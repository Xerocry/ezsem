#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
	printf("Parent pid %d, ppid %d.\n", getpid(), getppid());

	if(!fork()) {
		// Создаем процесс-потомок
		execl("p2.1.1.ch", "p2.1.1.ch", NULL);
	}	

	// Ожидаем завершение потомка
	int status;
	wait(&status);	

	printf("Parent has been finished.\n");

	return 0x0;
}
