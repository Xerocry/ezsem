#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define FIRST_DELAY 5
#define SECOND_DELAY 1

// Обработчик первого потока
void* firstThread() {
	while(1) {
		printf("First thread message.\n");
		sleep(FIRST_DELAY);
	}
}

// Обработчик второго потока
void* secondThread() {
	while(1) {
		printf("Second thread message.\n");
		sleep(SECOND_DELAY);
	}
}

int main() {
	pthread_t first, second;

	// Создание и запуск потоков
	pthread_create(&first, NULL, &firstThread, NULL);
	pthread_create(&second, NULL, &secondThread, NULL);

	// Ожидаем завершение потоков
	pthread_join(first, NULL);
	pthread_join(second, NULL);

	return 0x0;
}
