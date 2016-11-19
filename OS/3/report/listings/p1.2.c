#include <stdio.h>
#include <unistd.h>

int main() {
	// Обнуляем тестовую переменную
	int testValue = 0;
	printf("Process has been started #%d.\n", getpid());
	
	// Порождаем процесс-потомок
	if(fork()) {
		// Если родитель, то инкрементируем переменную
		++testValue;
		printf("Parent process #%d.\n", getpid());
		printf("Test value: %d \n", testValue);
	}
	else {
		// Если потомок, то декрементируем переменную
		--testValue;
		printf("Child process #%d.\n", getpid());
		printf("Test value: %d \n", testValue);
	}

	printf("Process has been finished #%d.\n", getpid());
	return 0x0;
}
