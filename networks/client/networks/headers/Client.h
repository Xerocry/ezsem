#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include <iostream>
#include <afxres.h>

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

    static constexpr const char* CLIENT_JOINED = "Client joined.";
    static constexpr const char* MESSAGE_RECEIVED = "Message received.";

    static const int EMPTY_FLAGS = 0;
    static const int MESSAGE_SIZE = 1000;

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
    const void start() const throw(ClientException);
    const void stop() throw(ClientException);
    ~Client();
};

#endif //NETWORKS_SERVER_H
