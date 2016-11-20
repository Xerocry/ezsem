#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>

#define THREAD_DELAY 1

void* threadHandler(void* threadNumber);

int main() {
	struct sched_param schedParam;
	pthread_t firstThread, secondThread, thirdThread;
	pthread_attr_t firstAttr, secondAttr, thirdAttr;
	int firstThreadNumber, secondThreadNumber, thirdThreadNumber;

	// Инициализация аттрибутов
	pthread_attr_init(&firstAttr);
	pthread_attr_init(&secondAttr);
	pthread_attr_init(&thirdAttr);

	// Установка политики планирования
	pthread_attr_setschedpolicy(&firstAttr, SCHED_FIFO);
	pthread_attr_setschedpolicy(&secondAttr, SCHED_RR);
	pthread_attr_setschedpolicy(&thirdAttr, SCHED_FIFO);
	
	// Установка приоритетов планирования
	schedParam.sched_priority = 1;
	pthread_attr_setschedparam(&firstAttr, &schedParam);
	schedParam.sched_priority = 1;
	pthread_attr_setschedparam(&secondAttr, &schedParam);
	schedParam.sched_priority = 99;
	pthread_attr_setschedparam(&thirdAttr, &schedParam);

	// Установка приоритетов для аттрибутов потоков
	pthread_attr_setinheritsched(&firstAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&secondAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&thirdAttr, PTHREAD_EXPLICIT_SCHED);

	// Установка приоритета наследования от родителя
	pthread_attr_setinheritsched(&firstAttr, PTHREAD_INHERIT_SCHED);

	// Создание и запуск потоков
	firstThreadNumber = 1;
	pthread_create(&firstThread, &firstAttr, &threadHandler, (void*) &firstThreadNumber);
	secondThreadNumber = 2;
	pthread_create(&secondThread, &secondAttr, &threadHandler, (void*) &secondThreadNumber);
	thirdThreadNumber = 3;
	pthread_create(&thirdThread, &thirdAttr, &threadHandler, (void*) &thirdThreadNumber);

	// Ожидаем завершение потоков
	pthread_join(firstThread, NULL);
	pthread_join(secondThread, NULL);
	pthread_join(thirdThread, NULL);

	// Удаляем аттрибуты
	pthread_attr_destroy(&firstAttr);
	pthread_attr_destroy(&secondAttr);
	pthread_attr_destroy(&thirdAttr);

	return 0x0;
}

void* threadHandler(void* threadNumber) {
	char* stringPolicy;

	//pthread_attr_t currentAttr = (*((int*)threadNumber) == 1) ? firstAttr: ((*((int*)threadNumber) == 2) ? secondAttr: thirdAttr);

	while(1) {
		//int currentPolicy;
		//pthread_attr_getschedpolicy(&currentAttr, &currentPolicy);

		switch(sched_getscheduler(0)) {
			case SCHED_FIFO: 
				stringPolicy = "SCHED_FIFO";
				break;
			case SCHED_RR: 
				stringPolicy = "SCHED_RR";
				break;
			case SCHED_OTHER: 
				stringPolicy = "SCHED_OTHER";
				break;
		}

		printf("Thread number %d, thread policy %s.\n", *((int*)threadNumber), stringPolicy);
		sleep(THREAD_DELAY);
	}

}
