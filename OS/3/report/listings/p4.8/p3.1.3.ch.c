#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handler(int signal);

int main() {
	printf("Third child pid %d, ppid %d.\n", getpid(), getppid());

	// Игнорирование сигнала
	signal(SIGUSR1, handler);

	// Бесконечный цикл, программа завершится только внешним сигналом
	while(1);

	return 0x0;
}

void handler(int signal) {
	printf("Signal handle");
	exit(0x0);
}

