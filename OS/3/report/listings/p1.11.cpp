#include <iostream>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include "singlecore.h"

const int DEFAULT_COEFF = 50;
const float SECONDS_COEFF = 1e9;

int main() {
	struct sched_param schedParam;
	struct timespec time;

	// Parent

	int pid = getpid();
	int ppid = getppid();

	std::cout << "Parent pid #" << pid << ", ppid #" << ppid << "." << std::endl;

	// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
	if(disableMultithreading(pid) < 0)
		return 0x1;

	// Записываем алгоритм планирования в поле структуры
	schedParam.sched_priority = DEFAULT_COEFF;

	// Вызов планировщика
	if(sched_setscheduler(0, SCHED_RR, &schedParam) == -1) {
		std::cerr << "It's impossible to set sheduler. Parent pid #" << pid << ", ppid #" << ppid << "." << std::endl;
		return 0x2;
	}

	// Определение длины кванта
	if(sched_rr_get_interval(0, &time) == 0) {
		std::cout << "Parent " << float(time.tv_nsec) / SECONDS_COEFF + time.tv_sec << " seconds." << std::endl;
	}
	else {
		std::cerr << "It's impossible to get sheduler interval. Parent pid #" << pid << ", ppid #" << ppid << "." << std::endl;
		return 0x3;
	}
		

	if(!fork()) {
		// Child

		pid = getpid();
		ppid = getppid();

		std::cout << "First child pid " << pid << ", ppid " << ppid << "." << std::endl;

		// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
		if(disableMultithreading(pid) < 0)
			return 0x3;

		// Определение длины кванта
		if(sched_rr_get_interval(0, &time) == 0) {
			std::cout << "Child " << float(time.tv_nsec) / SECONDS_COEFF + time.tv_sec << " seconds." << std::endl;
			execl("p1.9.2.c1", "p1.9.2.c1", NULL);
		}
		else {
			std::cerr << "It's impossible to get sheduler interval. Parent pid #" << pid << ", ppid #" << ppid << "." << std::endl;
			return 0x4;
		}
	}

	return 0x0;
}
