#include <iostream>
#include <unistd.h>
#include "../headers/Server.h"

static const uint16_t PORT = 65100;

int main(int argc, char *argv[]) {

    char* filename = (char*) "";
    if(argc > 2) {
        std::cerr << "Too many arguments." << std::endl;
        return 0xFF;
    }
    else if(argc == 2)
        filename = argv[0];

    try {
        Server(&std::cout, &std::cin, &std::cerr, PORT, filename).start();
    }
    catch (const Server::ServerException& exception) {
        std::cerr << exception.what() << std::endl;
        return exception.code();
    }
    catch (const Server::ServerController::ControllerException& exception) {
        std::cerr << exception.what() << std::endl;
        return exception.code();
    }

    return 0x0;
}