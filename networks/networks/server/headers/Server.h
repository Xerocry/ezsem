#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include "../headers/Global.h"

#include <stdint-gcc.h>
#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <mutex>

#ifdef _LINUX_
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#ifdef _WIN_
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
#endif

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
            USER_IS_NOT_CONNECTED_YET = 0x5,
            USER_IS_ALREADY_CONNECTED = 0x6,
            EVENT_IS_ALREADY_EXISTS = 0x7,
            EVENT_IS_NOT_EXISTS = 0x8,
            COULD_NOT_GET_CLIENT_INFO_BY_SOCKET = 0x9,
            WRONG_PASSWORD = 0xA,
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

#ifdef _LINUX_
        const std::pair<std::string, std::string> getAddressInfoBySocket(const int socket) const throw(ControllerException);
        const void close(const int socket) const throw(ControllerException);
        const void connect(const char* userName, const char* password, const int clientSocket) const throw(ControllerException);
        const char* getUserNameBySocket(const int clientSocket) const throw(ControllerException);
#endif
#ifdef _WIN_
        const std::pair<std::string, std::string> getAddressInfoBySocket(const SOCKET socket) const throw(ControllerException);
        const void close(const SOCKET socket) const throw(ControllerException);
        const void connect(const char* userName, const char* password, const SOCKET clientSocket) const throw(ControllerException);
        const char* getUserNameBySocket(const SOCKET clientSocket) const throw(ControllerException);
#endif

        const void finit(const char* filename) const;
        const void save() const throw(ControllerException);
        const void load() const throw(ControllerException);
        const void exit() const;
        const void eventCreate(const char* eventName, const std::chrono::milliseconds& start, const std::chrono::seconds& period) const throw(ControllerException);
        const void eventDrop(const char* eventName) const throw(ControllerException);
        const void eventSubscribe(const char* eventName, const char* userName) const throw(ControllerException);
        const void eventUnsubscribe(const char* eventName, const char* userName) const throw(ControllerException);
        const void eventNotify(const char *eventName) const;
        const void help(std::ostream* out) const;
        const void printSubscriptionsInfo() const;
        const void printSelfInfo(std::ostream* out, const char* userName) const;
        const void printUsersInfo() const;
        const void printEventsInfo(std::ostream* out) const;
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

#ifdef _LINUX_
        int socket = -1;
#endif
#ifdef _WIN_
        SOCKET socket = INVALID_SOCKET;
        bool clientInterrupt;
        bool serverInterrupt;
#endif

        sockaddr_in* address;
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
#ifdef _WIN_
        COULD_NOT_STARTUP = 0x8,
        COULD_NOT_RESOLVE_ADDRESS = 0x9,
#endif
    };

    static const int BACKLOG = 5;
    static const int EMPTY_FLAGS = 0;
    static const int MESSAGE_SIZE = 1000;

#ifdef _UDP_
    static constexpr const char* CONNECT_STRING = "*/*c*o*n*n*e*c*t*/*";
    static constexpr const char* ACCEPT_STRING = "*/*a*c*c*e*p*t*/*";
    static constexpr const char* DETACH_STRING = "*/*d*e*t*a*c*h*/*";
#endif

    static constexpr const char *DEFAULT_FILENAME = "server.data";
    static constexpr const char *BACKUP_FILENAME = "server.backup";

    std::shared_ptr<std::thread> commandThread;
    std::shared_ptr<std::thread> timerThread;

    std::mutex mutexFilename;
    std::string filename;

    std::mutex mutexAccounts;
    std::map<std::string, std::string> accounts;

    std::mutex mutexEvents;
    std::map<int, Server::Event*> events;

    std::mutex mutexUsers;
    std::map<int, Server::User*> users;

    std::mutex mutexSubscriptions;
    std::vector<std::pair<std::string, std::string>> subscriptions;

    std::mutex mutexTimings;
    std::vector<std::pair<int, std::chrono::milliseconds>> timings;

    std::mutex mutexIn;
    std::istream* in;

    std::mutex mutexOut;
    std::ostream* out;

    std::mutex mutexError;
    std::ostream* error;

    ServerController* controller;

    bool generalInterrupt, timerInterrupt;

    int generalBind = -1, generalFlags = -1;

#ifdef _LINUX_
    int generalSocket = -1;
#endif
#ifdef _WIN_
    SOCKET generalSocket = INVALID_SOCKET;
#endif

public:
    class ServerException: public std::exception {
    private:
        Error error;
    public:
        explicit ServerException(const Error);
        const char* what() const noexcept override;
        const int code() const;
    };

#ifdef _LINUX_
    explicit Server(std::ostream* out, std::istream* in, std::ostream* error, const uint16_t port, const char* filename) throw(ServerException);
#endif
#ifdef _WIN_
    explicit Server(std::ostream* out, std::istream* in, std::ostream* error, const char* port, const char* filename) throw(ServerException);
#endif

    const void start() throw(ServerException, ServerController::ControllerException);
    const void stop() throw(ServerException);
    ~Server();

private:
#ifdef _LINUX_
#ifdef _TCP_
    static void* clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket);
    const void acceptClient(const int threadId, const int clientSocket) throw(ServerException);
#endif
#ifdef _UDP_
    static void* clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket, const sockaddr_in* clientAddress);
    const void acceptClient(const int threadId, const int clientSocket, const sockaddr_in* clientAddress) throw(ServerException);
#endif

    const void createClientThread(const int clientSocket, sockaddr_in* clientAddress);
    const void removeClientThread(const int threadId) throw(ServerException);

    const void clearSocket(const int threadId, const int socket) throw(ServerException);
#ifdef _TCP_
    static const void writeLine(const std::string& message, const int socket) throw(ServerException);
#endif
#ifdef _UDP_
    static const void writeLine(const std::string& message, const int socket, const sockaddr_in* clientAddress) throw(ServerException);
#endif
#ifdef _TCP_
    static const std::string readLine(const int socket) throw(ServerException);
#endif
#ifdef _UDP_
    static const std::string readLine(const int socket, const sockaddr_in* clientAddress) throw(ServerException);
#endif
#endif
#ifdef _WIN_
    static void* clientThreadInitialize(void *thisPtr, const int threadId, const SOCKET clientSocket);
    const void acceptClient(const int threadId, const SOCKET clientSocket) throw(ServerException);

    const void createClientThread(const SOCKET clientSocket, sockaddr_in* clientAddress);
    const void removeClientThread(const int threadId) throw(ServerException);

    const void clearSocket(const int threadId, const SOCKET socket) throw(ServerException);

    static const void writeLine(const std::string& message, const SOCKET socket) throw(ServerException);
    const std::string readLine(const int threadId, const SOCKET socket) const throw(ServerException);
#endif

    static void* commandThreadInitialize(void *thisPtr);
    static void* timerThreadInitialize(void *thisPtr);
    const void eventTimer();
    const void commandExecutor();
    const void refreshTiming(const int eventId);

    const void lockAll(bool tryLockArray[9]);
    const void unlockAll(const bool tryLockArray[9]);
};

#endif //NETWORKS_SERVER_H
