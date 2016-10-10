#include <iostream>
#include <conio.h>
#include "../headers/Client.h"

static const char* PORT = "65100";
static const char* ADDRESS = "178.162.36.129";

int main(int argc, char *argv[]) {
    try {
        Client(&(std::cout), &(std::cin), PORT, ADDRESS).start();
    }
    catch (const Client::ClientException& exception) {
        std::cerr << exception.what() << std::endl << "Press any key to continue: ";
        _getch();
        return exception.code();
    }

    return 0;
}