#include "../headers/Client.h"

#ifdef _LINUX_
static const uint16_t PORT = 65100;
#endif
#ifdef _WIN_
static const char* PORT = "65100";
#endif
static const char* ADDRESS = "127.0.0.1";

int main(int argc, char *argv[]) {
    try { Client(&(std::cout), &(std::cin), PORT, ADDRESS).start(); }
    catch (const Client::ClientException& exception) {
        std::cerr << exception.what() << std::endl;
        std::getchar();
        return exception.code();
    }

    return 0;
}