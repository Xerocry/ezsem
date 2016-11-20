#include <unistd.h>
#include <signal.h>
#include <stdio.h>

int main() {
	printf("Parent pid %d, ppid %d.\n", getpid(), getppid());

	int firstPid, secondPid, thirdPid;

	if(!fork()) {
		firstPid = getpid();
		// Вызов программы, которая обрабатывает сигнал по умолчанию
		execl("p3.1.1.ch", "p3.1.1.ch", NULL);
	}
	
	if(!fork()) {
		secondPid = getpid();
		// Вызов программы, которая игнорирует посылаемый сигнал
		execl("p3.1.2.ch", "p3.1.2.ch", NULL);
	}

	if(!fork()) {
		thirdPid = getpid();
		// Вызов программы, с собственным обработчиком сигнала
		execl("p3.1.3.ch", "p3.1.3.ch", NULL);
	}

	// Запись таблицы процессов в файл
	system("ps -s > p3.1.0.log");

	// Посылаем сигнал первой программе
	kill(firstPid, SIGUSR1);
	// Запись таблицы процессов в файл
	system("ps -s > p3.1.1.log");

	// Посылаем сигнал второй программе
	kill(secondPid, SIGUSR1);
	// Запись таблицы процессов в файл
	system("ps -s > p3.1.2.log");

	// Посылаем сигнал третьей программе
	kill(thirdPid, SIGUSR1);
	// Запись таблицы процессов в файл
	system("ps -s > p3.1.3.log");

	return 0x0;
}
