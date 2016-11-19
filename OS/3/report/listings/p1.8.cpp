#include <unistd.h>
#include <iostream>
#include <string>

// Константы для функции семейства exec()
const char* FULL_PATH = "/bin/df";
const char* SIMPLE_PATH = "df";
const char* FIRST_ARG = "df";
const char* SECOND_ARG = "-h";
const char* LAST_ARG = (char*) NULL;

int main(int argc, char** argv) {
	// Проверка на количество аргументов, если неправильное, то выходим с ошибкой
	if(argc != 2) {
		std::cerr << "Invalid count of arguments." << std::endl;
		return 0x1;
	}

	int value;
	try { 
		value = std::stoi(argv[1]);
		if(value < 0 || value > 5)
			throw std::invalid_argument(argv[1]);
	}
	catch(const std::exception& exception) {
		// Обработка ситуации неверного аргумента
		std::cerr << "Invalid argument." << std::endl;
		return 0x2;	
	}

	// Массив аргументов
	char* args[] = {(char*) FIRST_ARG, (char*) SECOND_ARG, (char*) LAST_ARG};

	switch(value) {
		case 0:
			// Полный путь, аргументы программы в аргументах функции 
			execl(FULL_PATH, FIRST_ARG, SECOND_ARG, LAST_ARG);
			break;

		case 1: 
			// Неполный путь, аргументы программы в аргументах функции 
			execlp(SIMPLE_PATH, FIRST_ARG, SECOND_ARG, LAST_ARG);
			break;

		case 2: 
			// Полный путь, аргументы программы в аргументах функции, добавлен массив переменных окружения
			execle(FULL_PATH, FIRST_ARG, SECOND_ARG, LAST_ARG, __environ);
			break;
		case 3: 
			// Полный путь, аргументы программы указываются в массиве
			execv(FULL_PATH, args);
			break;
		case 4: 
			// Неполный путь, аргументы программы указываются в массиве
			execvp(SIMPLE_PATH, args);
			break;
		case 5: 
			// Неполный путь, аргументы программы указываются в массиве, добавлен массив переменных окружения
			execvpe(SIMPLE_PATH, args, __environ);			
			break;
		default: 
			// Обработка ситуации неверного аргумента
			std::cerr << "Impossible situation." << std::endl;
			return 0x3;
	}

	return 0x0;
}
