#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include <stdint-gcc.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <thread>

class Server {
public:
    class ServerController {
    private:
        Server* serverPtr;
    public:
        ServerController(Server* serverPtr);

        const void reg(const char* login, const char* password) const;
        const void del(const char* login, const char* password) const;
        const void detach(const char* login) const;
        const void save(const char* filename) const;
        const void load(const char* filename) const;
        const void exit() const;
        const void exit(const char* filename) const;
        const void eventCreate(const bool singleMode, const int period, const char* eventName) const;
        const void eventDrop(const char* eventName) const;
        const void eventSubscribe(const char* eventName, const char* login) const;
        const void eventUnsubscribe(const char* eventName, const char* login) const;
        const void eventNotify(const char *eventName) const;
        const void printUsersInfo() const;
        const void printEventsInfo() const;

        const int getThreadIdByLogin(const char* login) const;
        const char* getLoginByThreadId(const int threadId) const;
        const int getEventIdByEventName(const char* eventName) const;
        const char* getEventNameByEventId(const int eventId) const;
    };

private:
    enum Error {
        COULD_NOT_CREATE_SOCKET = 0x1,
        COULD_NOT_BIND = 0x2,
        COULD_NOT_SET_NON_BLOCKING = 0x3,
        COULD_NOT_ACCEPT = 0x4,
        COULD_NOT_RECEIVE_MESSAGE = 0x5,
        COULD_NOT_SHUT_SOCKET_DOWN = 0x6,
        COULD_NOT_CLOSE_SOCKET = 0x7,
    };

    class CommunicationConstants {
    public:
        static constexpr const char *CLIENT_JOINED = "Client joined.";
        static constexpr const char *MESSAGE_RECEIVED = "Message received.";
    };

    static const int BACKLOG = 5;
    static const int EMPTY_FLAGS = 0;
    static const int MESSAGE_SIZE = 1000;

    std::shared_ptr<std::thread> commandThread;
    std::map<int, std::shared_ptr<std::thread>> clientThreads;
    std::map<int, int> clientSockets;

    std::istream* in;
    std::ostream* out;
    std::ostream* error;

    ServerController* controller;

    bool clearStarted;
    int interrupt;

    int generalSocket = -1, generalBind = -1, generalFlags = -1;

public:
    class ServerException: public std::exception {
    private:
        Error error;
    public:
        explicit ServerException(const Error);
        const char* what() const noexcept override;
        const int code() const;
    };

    explicit Server(std::ostream* out, std::istream* in, std::ostream* error, const uint16_t port) throw(ServerException);
    const void start() throw(ServerException);
    const void stop() throw(ServerException);
    ~Server();

private:
    const void createClientThread(const int clientSocket, const struct in_addr address);
    const void removeClientThread(const int threadId) throw(ServerException);
    static void* clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket, const struct in_addr address);
    static void* commandThreadInitialize(void *thisPtr);
    const void commandExecutor();
    const void acceptClient(const int threadId, const int clientSocket, const struct in_addr address) throw(ServerException);
    const void clearSocket(const int threadId, const int socket) throw(ServerException);
};

#endif //NETWORKS_SERVER_H
