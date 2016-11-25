#include "../headers/Client.h"

#if defined(_LINUX_) || defined(_UDP_)
static const uint16_t PORT = 65100;
#endif
#if defined(_WIN_) && defined(_TCP_)
static const char* PORT = "65100";
#endif
static const char* ADDRESS = "127.0.0.1";

int main(int argc, char *argv[]) {

    std::string address = ADDRESS;
#if defined(_LINUX_) || defined(_UDP_)
    uint16_t port = PORT;
#endif
#if defined(_WIN_) && defined(_TCP_)
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

#if defined(_LINUX_) || defined(_UDP_)
        port = (uint16_t) std::atoi(argv[2]);
#endif
#if defined(_WIN_) && defined(_TCP_)
        port = argv[2];
#endif
    }
    
    try {
        Client(&(std::cout), &(std::cin), &(std::cerr),
#if defined(_LINUX_) || defined(_UDP_)
                port,
#endif
#if defined(_WIN_) && defined(_TCP_)
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