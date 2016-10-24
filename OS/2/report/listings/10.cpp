#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
	// Если нет аргументов командной строки
	if(argc < 2) {
		// Выводим сообщение об ошибке в поток ошибок и выходим с кодом 0x1
		std::cerr << "Error: No input file." << std::endl;
		return 0x1;
	}
	
	// Открываем поток на чтение из файла
	std::ifstream stream(argv[1]);
	
	// Если не удалось открыть файл
	if(!stream.is_open()) {
		// Выводим сообщение об ошибке в поток ошибок и выходим с кодом 0x2
		std::cerr << "Error: It's impossible to open file." << std::endl;
		return 0x2;
	}
	
	// Посимвольно читаем содержимое файла и выводим те же симводы в поток вывода
	char symbol;
	while(stream >> symbol)
		std::cout << symbol;

	std::cout << std::endl;

	// Закрываем поток чтения из файла
	stream.close();
	return 0x0;
}
