#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

int childPid;

void handler(int signal);

int main() {
	printf("Child pid %d, ppid %d.\n", getpid(), getppid());

	// Перевод программы в фоновый режим
	daemon(1, 1);
	
	// Определяем обработчик прерывания
	signal(SIGINT, handler);
	// Бесконечный цикл, программа завершится только внешним сигналом
	while(1);

	return 0x0;
}

void handler(int signal) {
	// Вывод текущего системного времени
	char buffer[100];
	struct timeval timeV;
	gettimeofday(&timeV, NULL);
	time_t currentTime = timeV.tv_sec;
	strftime(buffer, 100, "%T", localtime(&currentTime));
	printf("%s.%.3ld Child signal handle.\n", buffer, timeV.tv_usec);
	
	// Завершение программы
	exit(0x0);
}

