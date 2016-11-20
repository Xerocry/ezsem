#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

volatile sig_atomic_t childPid;

void handler(int signal);

int main() {
	printf("Parent pid %d, ppid %d.\n", getpid(), getppid());

	if(!(childPid = fork())) {
		// Вызываем дочерний процесс
		execl("p3.2.ch", "p3.2.ch", NULL);	
	} 
	else {
		// Определяем обработчик прерывания
		signal(SIGINT, handler);
		// Бесконечный цикл, программа завершится только внешним сигналом
		while(1);
	}

	return 0x0;
}

void handler(int signal) {
	// Вывод текущего системного времени
	char buffer[100];
	struct timeval timeV;
	gettimeofday(&timeV, NULL);
	time_t currentTime = timeV.tv_sec;
	strftime(buffer, 100, "%T", localtime(&currentTime));
	printf("%s.%.3ld Parent signal handle.\n", buffer, timeV.tv_usec);

	// Завершение программы
	exit(0x0);
}

