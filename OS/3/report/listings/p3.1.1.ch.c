#include <unistd.h>
#include <signal.h>
#include <stdio.h>

int main() {
	printf("First child pid %d, ppid %d.\n", getpid(), getppid());

	// Обработка сигнала по умолчанию
	signal(SIGINT, SIG_DFL);

	// Бесконечный цикл, программа завершится только внешним сигналом
	while(1);

	return 0x0;
}

