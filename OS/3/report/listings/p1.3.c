#include <stdio.h>
#include <unistd.h>
#include "singlecore.h"

#define ITERATIONS_COUNT ((int)1e6)
#define ITERATIONS_PERIOD ((int)1e5)

int main() {
	// Создаем временную переменную для идентификаторра процесса
	printf("Process has been starte.d #%d.\n", getpid());
	
	// Порождаем процесс-потомок
	if(fork()) {
		int pid = getpid();
		printf("Parent process #%d.\n", pid);

		// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
		if(disableMultithreading(pid) < 0)
			return 0x1;

		// Запускаем цикл с большим количеством итераций, и с каким то периодом выводим сообщение
		int iteration = ITERATIONS_COUNT;
		while(iteration != 0)
			if(--iteration % ITERATIONS_PERIOD == 0)
				printf("Parent win.\n");
	}
	else {
		int pid = getpid();
		printf("Child process #%d.\n", pid);

		// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
		if(disableMultithreading(pid) < 0)
			return 0x2;

		// Запускаем цикл с большим количеством итераций, и с каким то периодом выводим сообщение
		int iteration = ITERATIONS_COUNT;
		while(iteration != 0)
			if(--iteration % ITERATIONS_PERIOD == 0)
				printf("Child win.\n");
	}

	printf("Process has been finished #%d.\n", getpid());
	return 0x0;
}
