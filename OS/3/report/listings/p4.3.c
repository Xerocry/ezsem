#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define FIRST_DELAY 5
#define SECOND_DELAY 1
#define KILL_DELAY 5

int interrupt = 0;

void handler(int signal);
void* firstThread();
void* secondThread();

int main() {
	pthread_t first, second;

	// Создание и запуск первого потока
	pthread_create(&first, NULL, &firstThread, (void*) &second);

	// Создание и запуск второго потока
	pthread_create(&second, NULL, &secondThread, NULL);

	// Ожидаем завершение первого потока
	pthread_join(first, NULL);

	// Ожидаем завершение второго потока
	pthread_join(second, NULL);

	return 0x0;
}

void handler(int signal) {
	printf("Signal arrived.\n");
	// Прерываем второй поток
	interrupt = 1;
}

// Обработчик первого потока
void* firstThread(void* secondThread) {
	int pid = getpid();
	int ppid = getppid();

	sleep(KILL_DELAY);

	// Завершаем поток с сигналом SIGUSR1
	// Обращение к потоку происходят по указателю, перданному через аргумент функции
	pthread_kill(*((pthread_t*)secondThread), SIGUSR1);

	while(1) {
		printf("First thread message, pid %d, ppid %d.\n", pid, ppid);
		sleep(FIRST_DELAY);
	}
}

// Обработчик второго потока
void* secondThread() {
	int pid = getpid();
	int ppid = getppid();

	signal(SIGUSR1, handler);

	// Цикл работает пока не приходит прерывание
	while(!interrupt) {
		printf("Second thread message, pid %d, ppid %d.\n", pid, ppid);
		sleep(SECOND_DELAY);
	}

	printf("Second thread has been finished.\n");
	return NULL;
}
