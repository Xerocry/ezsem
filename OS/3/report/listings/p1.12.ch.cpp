#include <iostream>
#include <unistd.h>

#define FILE_LENGTH 15
#define STRING_LENGTH 15

int main(int argc, char* argv[]) {
	// Проверка на количество аргументов, если неправильное, то выходим с ошибкой
	if(argc > 2) {
		std::cerr << "Wrong count of arguments." << std::endl;
		return 0x1;	
	}

	int file;
	try { file = std::stoi(argv[1]); }
	catch(const std::exception& exception) {
		// Обработка ситуации неверного аргумента
		std::cerr << "Wrong argument." << std::endl;
		return 0x2;
	}

	std::cout << "Process pid " << getpid() << ", ppid " << getppid() << ". Read from file: ";

	// Считываем побайтово файл и выводим его содержимое 
	char symbol;
	int index = 0;
	while(index < FILE_LENGTH) {
		pread(file, &symbol, 1, index++);
		std::cout << symbol;
	}

	std::cout << std::endl;

	return 0x0;	
}
