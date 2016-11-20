#include <iostream>
#include <unistd.h>
#include <sched.h>
#include "singlecore.h"

const int DEFAULT_COEFF = 50;
const int DELAY = 30;
const int EXIT_DELAY = DELAY * 20;

void setPolicy(int* policy, const char* argument) throw(std::invalid_argument);

int main(int argc, char** argv) {
	// Проверка на количество аргументов, если неправильное, то выходим с ошибкой
	if(argc != 5) {
		std::cerr << "Invalid count of arguments." << std::endl;
		return 0x1;
	}

	int policyParent, policyChildFirst, policyChildSecond, policyChildThird;

	try {
		// Aлгоритм планирования родителя
		setPolicy(&policyParent, argv[1]);
		// Aлгоритм планирования первого потомка
		setPolicy(&policyChildFirst, argv[2]);
		// Aлгоритм планирования второго потомка
		setPolicy(&policyChildSecond, argv[3]);
		// Aлгоритм планирования третьего потомка
		setPolicy(&policyChildThird, argv[4]);
	} 
	catch(const std::exception& exception) {
		// Обработка ситуации неверного аргумента
		std::cerr << "Invalid argument." << std::endl;
		return 0x2;
	}

	struct sched_param schedParam;
	schedParam.sched_priority = DEFAULT_COEFF;

	// Parent

	int pid = getpid();
	int ppid = getppid();

	// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
	if(disableMultithreading(pid) < 0)
		return 0x3;

	// Записываем алгоритм планирования в поле структуры
	schedParam.sched_priority = (policyParent == SCHED_OTHER) ? 0 : DEFAULT_COEFF;

	// Вызов планировщика
	if(sched_setscheduler(0, policyParent, &schedParam) == -1) {
		std::cerr << "It's impossible to set sheduler. Parent pid #" << pid << ", ppid #" << ppid << "." << std::endl;
		return 0x4;
	}

	std::cout << "Parent pid #" << pid << ", ppid #" << ppid << "." << std::endl;

	if(!fork()) {
		// First child

		pid = getpid();
		ppid = getppid();

		// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
		if(disableMultithreading(pid) < 0)
			return 0x5;

		// Записываем алгоритм планирования в поле структуры
		schedParam.sched_priority = (policyChildFirst == SCHED_OTHER) ? 0 : DEFAULT_COEFF;
		// Вызов планировщика
		if(sched_setscheduler(pid, policyChildFirst, &schedParam) == -1) {
				std::cerr << "It's impossible to set sheduler. First child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
				return 0x6;
		}

		usleep(DELAY);

		std::cout << "First child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
	}

	if(!fork()) {
		// Second child

		pid = getpid();
		ppid = getppid();

		// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
		if(disableMultithreading(pid) < 0)
			return 0x7;

		// Записываем алгоритм планирования в поле структуры
		schedParam.sched_priority = (policyChildSecond == SCHED_OTHER) ? 0 : DEFAULT_COEFF;
		// Вызов планировщика
		if(sched_setscheduler(pid, policyChildSecond, &schedParam) == -1) {
				std::cerr << "It's impossible to set sheduler. Second child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
				return 0x8;
		}

		usleep(DELAY);

		std::cout << "Second child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
	}

	if(!fork()) {
		// Third child

		pid = getpid();
		ppid = getppid();

		// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
		if(disableMultithreading(pid) < 0)
			return 0x9;

		// Записываем алгоритм планирования в поле структуры
		schedParam.sched_priority = (policyChildThird == SCHED_OTHER) ? 0 : DEFAULT_COEFF;
		// Вызов планировщика
		if(sched_setscheduler(pid, policyChildThird, &schedParam) == -1) {
				std::cerr << "It's impossible to set sheduler. Third child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
				return 0xA;
		}

		usleep(DELAY);

		std::cout << "Third child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
	}

	usleep(EXIT_DELAY);
	
	return 0x0;
}

void setPolicy(int* policy, const char* argument) throw(std::invalid_argument) {
	switch(std::stoi(argument)) {
		case 0: *policy = SCHED_FIFO; break;
		case 1: *policy = SCHED_RR; break;
		case 2: *policy = SCHED_OTHER; break;
		default: throw std::invalid_argument(argument);
	}
}


