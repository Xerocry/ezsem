#include "../headers/Client.h"

#ifdef _LINUX_
static const uint16_t PORT = 65100;
#endif
#ifdef _WIN_
static const char* PORT = "65100";
#endif
static const char* ADDRESS = "127.0.0.1";

int main(int argc, char *argv[]) {

    std::string address = ADDRESS;
#ifdef _LINUX_
    uint16_t port = PORT;
#endif
#ifdef _WIN_
    std::string port = PORT;
#endif

    if(argc > 3) {
        std::cerr << "Too many arguments." << std::endl;
        return 0xFF;
    }
    else if(argc == 2)
        address = argv[1];
    else if(argc == 3) {
        address = argv[1];

#ifdef _LINUX_
        port = (uint16_t) std::atoi(argv[2]);
#endif
#ifdef _WIN_
        port = argv[2];
#endif
    }
    
    try {
        Client(&(std::cout), &(std::cin),
#ifdef _LINUX_
                port,
#endif
#ifdef _WIN_
                port.data(),
#endif
               address.data()).start();
    }
    catch (const Client::ClientException& exception) {
        std::cerr << exception.what() << std::endl;
        std::getchar();
        return exception.code();
    }

    return 0;
}