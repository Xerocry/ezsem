#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define FIRST_DELAY 1
#define SIGNAL_COUNT 5

int count = 0;

void handler(int signalCode);
void* firstThread();

int main() {
	pthread_t first;

	// Создание и запуск первого потока
	pthread_create(&first, NULL, &firstThread, NULL);

	// Ожидаем завершение первого потока
	pthread_join(first, NULL);

	return 0x0;
}

void handler(int signalCode) {
	printf("Signal arrived. Counter: %d.\n", ++count);

	// Если количество прерываний больше критического значения, то воостанавливаем обработчик по умолчанию
	if(count >= SIGNAL_COUNT)
		signal(SIGTSTP, SIG_DFL);
}

// Обработчик первого потока
void* firstThread(void* secondThread) {
	int pid = getpid();
	int ppid = getppid();

	signal(SIGTSTP, handler);

	while(1) {
		printf("First thread message, pid %d, ppid %d.\n", pid, ppid);
		sleep(FIRST_DELAY);
	}
}
