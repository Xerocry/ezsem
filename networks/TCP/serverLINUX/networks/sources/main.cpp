#include <iostream>
#include "../headers/Server.h"

const uint16_t PORT = 65100;

int main(int argc, char *argv[]) {
    try {
        Server(&std::cout, &std::cin, &std::cerr, PORT).start();
    }
    catch (const Server::ServerException& exception) {
        std::cerr << exception.what() << std::endl;
        return exception.code();
    }

    return 0;
}