#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

const int CHILD_PROCESS_COUNT = 3;

int main(int argc, char** argv) {
	// Проверка на количество аргументов, если неправильное, то выходим с ошибкой
	if(argc != 2) {
		std::cerr << "Invalid count of arguments." << std::endl;
		return 0x1;
	}

	int value;
	try { 
		value = std::stoi(argv[1]);
		if(value < 0 || value > 3)
			throw std::invalid_argument(argv[1]);
	}
	catch(const std::exception& exception) {
		// Обработка ситуации неверного аргумента
		std::cerr << "Invalid argument." << std::endl;
		return 0x2;	
	}

	// Задаем флаги исполнения в зависимости от аргумента
	int* option;
	switch(value) {
		case 0:
			option = new int[CHILD_PROCESS_COUNT]{WNOHANG, WUNTRACED, WNOHANG};
			break;
		case 1:
			option = new int[CHILD_PROCESS_COUNT]{WUNTRACED, WUNTRACED, WUNTRACED};
			break;
		case 2:
			option = new int[CHILD_PROCESS_COUNT]{WNOHANG, WUNTRACED, WUNTRACED};
			break;
		default: 
			// Обработка ситуации неверного аргумента
			std::cerr << "Impossible situation." << std::endl;
			return 0x3;
	}

	int childPid, childWait, childStatus;
	const char* childProgramName[] = {"p1.9.2.c1", "p1.9.2.c2", "p1.9.2.c3"};

	for(int index = 0; index < CHILD_PROCESS_COUNT; ++index) {
		// Порождаем процесс-потомок
		childPid = fork();
		if(!childPid) {
			// Запускаем программу
			execl(childProgramName[index], childProgramName[index], NULL);
			return 0x0;		
		}

		// Выводим дерево процессов в файл
		system("ps xf > p1.9.2.p.ps.log");

		// Ждем выполнения процесса-потомка
		childWait = waitpid(childPid, &childStatus, option[index]);
		std::cout << "Child process #" << index + 1 << " has been finished ";

		// Проверка на успешность решения процесса
		if(WIFEXITED(childStatus) == 0)
			std::cout << " unsuccessfully." << std::endl;		
		else
			std::cout << " successfully." << std::endl;		
	}

	return 0x0;
}
