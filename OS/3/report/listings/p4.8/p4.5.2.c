#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define FIRST_DELAY 1

int interrupt = 0;

void handler(int signalCode);
void* firstThread();

int main() {
	pthread_t first;

	// Создание и запуск первого потока
	pthread_create(&first, NULL, &firstThread, NULL);

	// Ожидаем завершение первого потока
	pthread_join(first, NULL);

	printf("Program has been finished.\n");
	return 0x0;
}

void handler(int signalCode) {
	printf("Signal arrived.\n");

	interrupt = 1;
}

// Обработчик первого потока
void* firstThread(void* secondThread) {
	int pid = getpid();
	int ppid = getppid();

	signal(SIGINT, handler);

	while(!interrupt) {
		printf("First thread message, pid %d, ppid %d.\n", pid, ppid);
		sleep(FIRST_DELAY);
	}

	printf("Thread has been finished.\n");
	return NULL;
}
