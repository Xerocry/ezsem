#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int signalCode);

int main() {
	printf("Process started, pid %d, ppid %d.\n", getpid(), getppid());

	// Устанавливаем обработчик прерывания
	signal(SIGINT, handler);

	// Выход из приложения только по внешнему прерыванию
	while(1);

	return 0x0;
}

void handler(int signalCode) {
	printf("Signal arrived.\n");
	exit(0x0);
}
