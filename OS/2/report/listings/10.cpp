#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
	if(argc < 2) {
		std::cerr << "Error: No input file." << std::endl;
		return 0x1;
	}
	std::ifstream stream(argv[1]);
	if(!stream.is_open()) {
		std::cerr << "Error: It's impossible to open file." << std::endl;
		return 0x2;
	}
	
	char symbol;
	while(stream >> symbol)
		std::cout << symbol;

	std::cout << std::endl;

	stream.close();
	return 0x0;
}
