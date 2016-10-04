#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include <stdint-gcc.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <thread>
#include <vector>

class Server {
public:
    class ServerController {
    private:
        Server* serverPtr;

    public:
        enum Error {
            COULD_NOT_OPEN_FILE = 0x1,
            COULD_NOT_PARSE_FILE = 0x2,
            USER_IS_ALREADY_EXISTS = 0x3,
            USER_IS_NOT_EXISTS = 0x4,
            EVENT_IS_ALREADY_EXISTS = 0x5,
            EVENT_IS_NOT_EXISTS = 0x6
        };

        class ControllerException: public std::exception {
        private:
            Error error;
        public:
            explicit ControllerException(const Error);
            const char* what() const noexcept override;
            const int code() const;
        };

        ServerController(Server* serverPtr);

        const void reg(const char* userName, const char* password) const throw(ControllerException);
        const void del(const char* userName) const throw(ControllerException);
        const void detach(const char* userName) const throw(ControllerException);
        const void finit(const char* filename) const;
        const void save() const throw(ControllerException);
        const void load() const throw(ControllerException);
        const void exit() const;
        const void eventCreate(const char* eventName, const bool singleMode, const std::chrono::milliseconds& start, const std::chrono::seconds& period) const throw(ControllerException);
        const void eventDrop(const char* eventName) const throw(ControllerException);
        const void eventSubscribe(const char* eventName, const char* userName) const throw(ControllerException);
        const void eventUnsubscribe(const char* eventName, const char* userName) const throw(ControllerException);
        const void eventNotify(const char *eventName) const;

        const void printSubscriptionsInfo() const;
        const void printUsersInfo() const;
        const void printEventsInfo() const;
        const void printAccountsInfo() const;

        const int getThreadIdByUserName(const char* userName) const throw(ControllerException);
        const char* getUserNameByThreadId(const int threadId) const throw(ControllerException);
        const int getEventIdByEventName(const char* eventName) const throw(ControllerException);
        const char* getEventNameByEventId(const int eventId) const throw(ControllerException);
    };

    struct Event {
        std::string eventName;
        std::chrono::milliseconds startMoment;
        std::chrono::seconds period;
    };
    struct User{
        std::string userName;
        std::shared_ptr<std::thread> thread;
        int socket;
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

    static constexpr const char *DEFAULT_FILENAME = "server.data";
    static constexpr const char *BACKUP_FILENAME = "server.backup";

    std::shared_ptr<std::thread> commandThread;
    std::shared_ptr<std::thread> timerThread;

    std::string filename;

    std::map<std::string, std::string> accounts;
    std::map<int, Server::Event*> events;
    std::map<int, Server::User*> users;
    std::vector<std::pair<int, std::chrono::milliseconds>> timings;
    std::vector<std::pair<std::string, std::string>> subscriptions;

    std::istream* in;
    std::ostream* out;
    std::ostream* error;

    ServerController* controller;

    bool generalInterrupt, timerInterrupt;

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

    explicit Server(std::ostream* out, std::istream* in, std::ostream* error, const uint16_t port, const char* filename) throw(ServerException);
    const void start() throw(ServerException, ServerController::ControllerException);
    const void stop() throw(ServerException);
    ~Server();

private:
    const void createClientThread(const int clientSocket, const struct in_addr address);
    const void removeClientThread(const int threadId) throw(ServerException);
    static void* clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket, const struct in_addr address);
    static void* commandThreadInitialize(void *thisPtr);
    static void* timerThreadInitialize(void *thisPtr);
    const void eventTimer();
    const void commandExecutor();
    const void refreshTiming(const int eventId);
    const void acceptClient(const int threadId, const int clientSocket, const struct in_addr address) throw(ServerException);
    const void clearSocket(const int threadId, const int socket) throw(ServerException);
};

#endif //NETWORKS_SERVER_H
