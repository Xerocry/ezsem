#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include <stdint-gcc.h>
#include <iostream>
#include <netinet/in.h>
#include <map>
#include <thread>

class Server {
private:
    enum Error {
        COULD_NOT_CREATE_SOCKET = 0x1,
        COULD_NOT_BIND = 0x2,
        COULD_NOT_ACCEPT = 0x3,
        COULD_NOT_RECEIVE_MESSAGE = 0x4,
        COULD_NOT_SHUT_SOCKET_DOWN = 0x5,
        COULD_NOT_CLOSE_SOCKET = 0x6,
    };

    static constexpr const char* CLIENT_JOINED = "Client joined.";
    static constexpr const char* MESSAGE_RECEIVED = "Message received.";

    static const int BACKLOG = 5;
    static const int EMPTY_FLAGS = 0;
    static const int MESSAGE_SIZE = 1000;

    std::map<int, std::shared_ptr<std::thread>> threads;

    std::ostream* out;
    std::ostream* error;
    int generalSocket = -1, generalBind = -1;

public:
    class ServerException: public std::exception {
    private:
        Error error;
    public:
        ServerException(const Error);
        const char* what() const noexcept;
        const int code() const;
    };

    explicit Server(std::ostream* out, std::ostream* error, const uint16_t port) throw(ServerException);
    const void start() throw(ServerException);
    const void stop() throw(ServerException);
    const void createThread(const int clientAccept, const struct in_addr address);
    const void removeThread(const int threadId);
    ~Server();

private:
    static void* threadInitialize(void* thisPtr, const int threadId, const int clientAccept, const struct in_addr address);
    const void acceptClient(const int threadId, const int clientAccept, const struct in_addr address) throw(ServerException);
};

#endif //NETWORKS_SERVER_H
