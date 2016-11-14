#include <iostream>
#include <string>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include "singlecore.h"

int main(int argc , char** argv) {
	// Проверка на количество аргументов, если неправильное, то выходим с ошибкой
	if(argc != 6) {
		std::cerr << "Invalid count of arguments." << std::endl;
		return 0x1;
	}

	int policy;
	int coeffParent, coeffChildFirst, coeffChildSecond, coeffChildThird;

	try {
		// Первый аргумент - алгоритм планирования
		switch(std::stoi(argv[1])) {
			case 0: policy = SCHED_FIFO; break;
			case 1: policy = SCHED_RR; break;
			case 2: policy = SCHED_OTHER; break;
			default: throw std::invalid_argument(argv[2]);
		}
	
		// Следующие четыре аргумента - приоритеты
		coeffParent = std::stoi(argv[2]);
		coeffChildFirst = std::stoi(argv[3]);
		coeffChildSecond = std::stoi(argv[4]);
		coeffChildThird = std::stoi(argv[5]);
	} 
	catch(const std::exception& exception) {
		// Обработка ситуации неверного аргумента
		std::cerr << "Invalid argument." << std::endl;
		return 0x2;
	}

	struct sched_param schedParam;
	schedParam.sched_priority = 0;

	// Parent
	
	int pid = getpid();
	int ppid = getppid();

	// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
	if(disableMultithreading(pid) < 0)
		return 0x3;

	// Записываем алгоритм планирования в поле структуры
	if(policy != SCHED_OTHER)
		schedParam.sched_priority = coeffParent;
	// Вызов планировщика
	if(sched_setscheduler(0, policy, &schedParam) == -1) {
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
		if(policy != SCHED_OTHER)
			schedParam.sched_priority = coeffChildFirst;
		// Вызов планировщика
		if(sched_setscheduler(pid, policy, &schedParam) == -1) {
				std::cerr << "It's impossible to set sheduler. First child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
				return 0x6;
		}

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
		if(policy != SCHED_OTHER)
			schedParam.sched_priority = coeffChildSecond;
		// Вызов планировщика
		if(sched_setscheduler(pid, policy, &schedParam) == -1) {
				std::cerr << "It's impossible to set sheduler. Second child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
				return 0x8;
		}

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
		if(policy != SCHED_OTHER)
			schedParam.sched_priority = coeffChildThird;
		// Вызов планировщика
		if(sched_setscheduler(pid, policy, &schedParam) == -1) {
				std::cerr << "It's impossible to set sheduler. Third child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
				return 0xA;
		}

		std::cout << "Third child pid #" << pid << ", ppid #" << ppid << "." << std::endl;
	}
	
	std::cin.get();
	return 0x0;
}
