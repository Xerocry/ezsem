#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include "../headers/Global.h"

#ifdef _LINUX_
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#ifdef _WIN_
// For getaddrinfo() definition.

#include <windef.h>

#if(_WIN32_WINNT < 0x0501)
    #define _WIN32_WINNT_TEMP _WIN32_WINNT
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
#endif

#include <ws2tcpip.h>

#ifdef _WIN32_WINNT_TEMP
    #undef _WIN32_WINNT
    #define _WIN32_WINNT _WIN32_WINNT_TEMP
    #undef _WIN32_WINNT_TEMP
#endif

// End getaddrinfo() include.

#include <w32api/inaddr.h>

#endif

#include <iostream>
#include <thread>

class Client {
private:
    enum Error {
        COULD_NOT_STARTUP = 0x1,
        COULD_NOT_RESOLVE_ADDRESS = 0x2,
        COULD_NOT_CREATE_SOCKET = 0x3,
        COULD_NOT_CREATE_CONNECTION = 0x4,
        COULD_NOT_SEND_MESSAGE = 0x5,
        COULD_NOT_RECEIVE_MESSAGE = 0x6,
        COULD_NOT_SHUT_SOCKET_DOWN = 0x7,
        COULD_NOT_CLOSE_SOCKET = 0x8,
    };

    static const int EMPTY_FLAGS = 0;
    static const int MESSAGE_SIZE = 1000;

#ifdef _UDP_
    static constexpr const char* SEND_STRING = "@S";
    static constexpr const char* RESPONSE_STRING = "@R";
    static constexpr const char* ATTACH_STRING = "@A";
    static constexpr const char* DETACH_STRING = "@D";
#endif

    std::shared_ptr<std::thread> readThread;

    std::ostream* out;
    std::istream* in;

    bool globalInterrupt;

    int generalWSAStartup = -1;

#ifdef _UDP_
    struct sockaddr_in serverAddress;
#endif

#ifdef _LINUX_
    int generalSocket = -1;
#endif
#ifdef _WIN_
    SOCKET generalSocket = INVALID_SOCKET;
#endif

public:
    class ClientException: public std::exception {
    private:
        Error error;
    public:
        explicit ClientException(const Error);
        const char* what() const noexcept;
        const int code() const;
    };


#if defined(_LINUX_) || defined(_UDP_)
    explicit Client(std::ostream* out, std::istream* in,  const uint16_t port, const char* address) throw(ClientException);
#endif
#if defined(_WIN_) && defined(_TCP_)
    explicit Client(std::ostream* out, std::istream* in,  const char* port, const char* address) throw(ClientException);
#endif
    const void start() throw(ClientException);

    static void* readThreadInitialize(void *thisPtr);
    const void feedbackExecutor();
#ifdef _UDP_
    const void writeLine() throw(ClientException);
#endif
    const std::string readLine() throw(ClientException);

    const void stop() throw(ClientException);
    ~Client();
};

#endif //NETWORKS_SERVER_H
