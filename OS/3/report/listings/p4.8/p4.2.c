#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define FIRST_DELAY 5
#define SECOND_DELAY 1

void* firstThread();
void* secondThread();

int main() {
	pthread_t first, second;

	system("ps -axhf > p4.2.1.log");

	// Создание и запуск первого потока
	pthread_create(&first, NULL, &firstThread, NULL);
	system("ps -axhf > p4.2.2.log");

	// Создание и запуск второго потока
	pthread_create(&second, NULL, &secondThread, NULL);
	system("ps -axhf > p4.2.3.log");

	// Ожидаем завершение первого потока
	pthread_join(first, NULL);

	// Ожидаем завершение второго потока
	pthread_join(second, NULL);

	return 0x0;
}

// Обработчик первого потока
void* firstThread() {
	int pid = getpid();
	int ppid = getppid();

	while(1) {
		printf("First thread message, pid %d, ppid %d.\n", pid, ppid);
		sleep(FIRST_DELAY);
	}
}

// Обработчик второго потока
void* secondThread() {
	int pid = getpid();
	int ppid = getppid();

	while(1) {
		printf("Second thread message, pid %d, ppid %d.\n", pid, ppid);
		sleep(SECOND_DELAY);
	}
}
