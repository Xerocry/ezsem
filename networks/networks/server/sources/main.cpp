//#include <iostream>
#include "../headers/Server.h"

#ifdef _LINUX_
static const uint16_t PORT = 65100;
#endif
#ifdef _WIN_
static const char* PORT = "65100";
#endif

int main(int argc, char *argv[]) {

    char* filename = (char*) "";
    if(argc > 2) {
        std::cerr << "Too many arguments." << std::endl;
        return 0xFF;
    }
    else if(argc == 2)
        filename = argv[1];

    try {

        Server(&std::cout, &std::cin, &std::cerr, PORT, filename).start();
    }
    catch (const Server::ServerException& exception) {
        std::cerr << exception.what() << std::endl;
        std::getchar();
        return exception.code();
    }
    catch (const Server::ServerController::ControllerException& exception) {
        std::cerr << exception.what() << std::endl;
        std::getchar();
        return exception.code();
    }

    return 0x0;
}