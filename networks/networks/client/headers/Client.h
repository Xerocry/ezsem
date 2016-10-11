#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include "../headers/Global.h"

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
    };

    static const int EMPTY_FLAGS = 0;
    static const int MESSAGE_SIZE = 1000;

    std::shared_ptr<std::thread> readThread;

    std::ostream* out;
    std::istream* in;

    int generalWSAStartup = -1;
    SOCKET generalSocket = INVALID_SOCKET;

public:
    class ClientException: public std::exception {
    private:
        Error error;
    public:
        explicit ClientException(const Error);
        const char* what() const noexcept;
        const int code() const;
    };

    explicit Client(std::ostream* out, std::istream* in,  const char* port, const char* address) throw(ClientException);
    const void start() throw(ClientException);

    static void* readThreadInitialize(void *thisPtr);
    const void feedbackExecutor() const;

#ifdef _LINUX_
    static const std::string readLine(const int socket) throw(ServerException);
#endif
#ifdef _WIN_
    static const std::string readLine(const SOCKET socket) throw(ClientException);
#endif
    const void stop() throw(ClientException);
    ~Client();
};

#endif //NETWORKS_SERVER_H
