#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define DELAY 7

int main() {
	printf("Second child pid %d, ppid %d.\n", getpid(), getppid());

	// Игнорирование сигнала
	signal(SIGINT, SIG_IGN);

	// Задержка перед выходом программы
	sleep(DELAY);

	return 0x0;
}

