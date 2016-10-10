#include "../headers/ServerCommand.h"
#include "../headers/ClientCommand.h"

#ifdef _LINUX_
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef _LINUX_
Server::Server(std::ostream* out, std::istream* in, std::ostream* error, const uint16_t port, const char* filename) throw(ServerException) {
#endif
#ifdef _WIN_
Server::Server(std::ostream* out, std::istream* in, std::ostream* error, const char* port, const char* filename) throw(ServerException) {
#endif

    this->out = out;
    this->in = in;
    this->error = error;
    this->filename = std::string(filename);

#ifdef _LINUX_
    generalSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(generalSocket < 0)
        throw ServerException(COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    generalBind = bind(generalSocket, (sockaddr *) &serverAddress, sizeof(serverAddress));
    if(generalBind < 0)
        throw ServerException(COULD_NOT_BIND);

    *this->out << "Socket has been successfully bind." << std::endl;

    generalFlags = fcntl(generalSocket, F_GETFL, 0);
    auto fcntlResult = fcntl (generalSocket, F_SETFL, generalFlags | O_NONBLOCK);
    if (fcntlResult < 0)
        throw ServerException(COULD_NOT_SET_NON_BLOCKING);

    listen(generalSocket, BACKLOG);

    *this->out << "Listen socket." << std::endl;
#endif
#ifdef _WIN_
    WSADATA wsaData;
    auto wsaStartup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(wsaStartup != 0)
        throw ServerException(COULD_NOT_STARTUP);

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *addressResult = nullptr;
    auto wsaAddress = getaddrinfo(NULL, port, &hints, &addressResult);
    if (wsaAddress != 0)
        throw ServerException(COULD_NOT_RESOLVE_ADDRESS);

    generalSocket = socket(addressResult->ai_family, addressResult->ai_socktype, addressResult->ai_protocol);
    if(generalSocket == INVALID_SOCKET)
        throw ServerException(COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

    generalBind = bind(generalSocket, addressResult->ai_addr, (int) addressResult->ai_addrlen);
    if(generalBind == SOCKET_ERROR)
        throw ServerException(COULD_NOT_BIND);

    *this->out << "Socket has been successfully bind." << std::endl;

    listen(generalSocket, BACKLOG);

    *this->out << "Listen socket." << std::endl;

    unsigned int iMode = 1;
    generalFlags = ioctlsocket(generalSocket, 0x8004667E, &iMode);
    if(generalFlags == SOCKET_ERROR)
        throw ServerException(COULD_NOT_SET_NON_BLOCKING);
#endif
}

const void Server::start() throw(ServerException, ServerController::ControllerException) {
    this->events = std::map<int, Server::Event*>();
    this->users = std::map<int, Server::User*>();
    this->accounts = std::map<std::string, std::string>();
    this->timings = std::vector<std::pair<int, std::chrono::milliseconds>>();
    this->subscriptions = std::vector<std::pair<std::string, std::string>>();

    this->generalInterrupt = false;

    this->controller = new ServerController(this);
    this->controller->load();

    auto bind = std::bind(&Server::commandThreadInitialize, std::placeholders::_1);
    commandThread = std::make_shared<std::thread>(bind, this);

    while(!this->generalInterrupt) {
        auto clientAddress = new sockaddr_in;
        auto size = sizeof(struct sockaddr_in);
        auto clientSocket = accept(generalSocket, (sockaddr *) clientAddress, (socklen_t *) &size);
#ifdef _LINUX_
        if (clientSocket >= 0)
#endif
#ifdef _WIN_
        if (clientSocket != INVALID_SOCKET)
#endif
            createClientThread(clientSocket, clientAddress);
        else
            delete clientAddress;
    }
}

void* Server::timerThreadInitialize(void *thisPtr) {
    ((Server*)thisPtr)->eventTimer();
    return NULL;
}

const void Server::eventTimer() {
    while(!this->generalInterrupt && !this->timerInterrupt) {
        bool lockTimings = this->mutexTimings.try_lock();

        if(this->timings.empty()) {
            if(lockTimings)
                this->mutexTimings.unlock();

            continue;
        }

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

        auto resultVector = std::vector<int>();

        for(auto& current: this->timings) {
            if (now >= current.second)
                resultVector.push_back(current.first);
            else
                break;
        }

        if(lockTimings)
            this->mutexTimings.unlock();

        for(auto& current: resultVector) {
            refreshTiming(current);
            this->controller->eventNotify(this->controller->getEventNameByEventId(current));
        }
    }
}

void* Server::commandThreadInitialize(void *thisPtr) {
    ((Server*)thisPtr)->commandExecutor();
    return NULL;
}

const void Server::commandExecutor() {
    std::string command;
    while(!this->generalInterrupt) {
        bool lockIn = this->mutexIn.try_lock();
        std::getline(*this->in, command);
        if(lockIn)
            this->mutexIn.unlock();

        bool lockOut, lockError;

        try {
            ServerCommand(&command, this->controller).parseAndExecute();

            lockOut = this->mutexOut.try_lock();
            *this->out << "Command has been successfully executed." << std::endl;
            if(lockOut)
                this->mutexOut.unlock();
        }
        catch(const Command::CommandException& exception) {
            lockError = this->mutexError.try_lock();
            *this->error << exception.what() << std::endl;
            if(lockError)
                this->mutexError.unlock();
        }
        catch(const Server::ServerController::ControllerException& exception) {
            lockError = this->mutexError.try_lock();
            *this->error << exception.what() << std::endl;
            if(lockError)
                this->mutexError.unlock();
        }
        catch(const std::exception& exception) {
            lockError = this->mutexError.try_lock();
            *this->error << exception.what() << std::endl;
            if(lockError)
                this->mutexError.unlock();
        }
        catch(...) {
            lockError = this->mutexError.try_lock();
            *this->error << "Strange resolve command error." << std::endl;
            if(lockError)
                this->mutexError.unlock();
        }
    }
}

const void Server::refreshTiming(const int eventId) {
    bool lockTimings = this->mutexTimings.try_lock();

    std::chrono::milliseconds timing;
    bool eraseCompleted = false;

    for(auto current = this->timings.begin(); current != this->timings.end(); ++current)
        if(current->first == eventId) {
            timing = current->second;
            this->timings.erase(current);
            eraseCompleted = true;
            break;
        }

    bool lockEvents = this->mutexEvents.try_lock();

    if(this->events.find(eventId) == this->events.end()) {
        if(lockEvents)
            this->mutexEvents.unlock();
        if(lockTimings)
            this->mutexTimings.unlock();
        return;
    }

    auto event = this->events.at(eventId);

    if(eraseCompleted) {
        if(event->period.count() == 0) {
            this->events.erase(eventId);

            if(lockEvents)
                this->mutexEvents.unlock();
            if(lockTimings)
                this->mutexTimings.unlock();
            return;
        }
        timing += event->period;
    }
    else
        timing = event->startMoment;

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    auto buffer = Command::EVENT_BUFFER;

    if(now + std::chrono::seconds(buffer) >= timing) {
        if(event->period.count() == 0) {
            this->events.erase(eventId);

            if(lockEvents)
                this->mutexEvents.unlock();
            if(lockTimings)
                this->mutexTimings.unlock();
            return;
        }
        else {
            auto count = (now + std::chrono::seconds(buffer) - timing) / event->period;
            timing += (count + 1) * event->period;
        }
    }

    if(lockEvents)
        this->mutexEvents.unlock();

    bool insertCompleted = false;

    for(auto current = this->timings.begin(); current != this->timings.end(); ++current)
        if(current->second >= timing) {
            this->timings.insert(current, std::pair<int, std::chrono::milliseconds>(eventId, timing));
            insertCompleted = true;
            break;
        }

    if(!insertCompleted)
        this->timings.insert(this->timings.end(), std::pair<int, std::chrono::milliseconds>(eventId, timing));

    if(lockTimings)
        this->mutexTimings.unlock();
}

#ifdef _LINUX_
void* Server::clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket) {
#endif
#ifdef _WIN_
void* Server::clientThreadInitialize(void *thisPtr, const int threadId, const SOCKET clientSocket) {
#endif
    auto serverPtr = ((Server*)thisPtr);

    bool lockOut, lockError;

    try { serverPtr->acceptClient(threadId, clientSocket); }
    catch (const ServerException& exception) {
        lockError = serverPtr->mutexError.try_lock();
        *serverPtr->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
        if(lockError)
            serverPtr->mutexError.unlock();
    }
    try {
        serverPtr->removeClientThread(threadId);

        lockOut = serverPtr->mutexOut.try_lock();
        *serverPtr->out << "Thread 0x" << threadId << ". Client disconnected." << std::endl;
        if(lockOut)
            serverPtr->mutexOut.unlock();
    } catch (const ServerException& exception) {
        lockError = serverPtr->mutexError.try_lock();
        *serverPtr->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
        if(lockError)
            serverPtr->mutexError.unlock();
    }

    return NULL;
}
#ifdef _LINUX_
const void Server::acceptClient(const int threadId, const int clientSocket) throw(ServerException) {
#endif
#ifdef _WIN_
const void Server::acceptClient(const int threadId, const SOCKET clientSocket) throw(ServerException) {
#endif
    bool lockOut = this->mutexOut.try_lock();
    *this->out << "Thread 0x" << threadId << ". Client has been connected." << std::endl;
    if(lockOut)
        this->mutexOut.unlock();

    while(!this->generalInterrupt) {
        std::string message;
        try{ message = readLine(threadId, clientSocket); }
        catch (const ServerException& exception) {
            if(exception.code() == COULD_NOT_RECEIVE_MESSAGE)
                break;
        }

        auto stream = new std::stringstream();
        try {
            ClientCommand(&message, this->controller, clientSocket).parseAndExecute(stream);
            *stream << "Command has been successfully executed." << std::endl;
        }
        catch(const Command::CommandException& exception) {
            *stream << exception.what() << std::endl;
        }
        catch(const Server::ServerController::ControllerException& exception) {
            *stream << exception.what() << std::endl;
        }
        catch(const std::exception& exception) {
            *stream << exception.what() << std::endl;
        }
        catch(...) {
            *stream << "Strange resolve command error." << std::endl;
        }

        writeLine(stream->str(), clientSocket);
    }
}

#ifdef _LINUX_
const void Server::writeLine(const std::string& message, const int socket) throw(ServerException) {
#endif
#ifdef _WIN_
const void Server::writeLine(const std::string& message, const SOCKET socket) throw(ServerException) {
#endif
    if(message.empty())
        return;

    auto output = message.data();

    send(socket, output, (int) strlen(output), EMPTY_FLAGS);
}

#ifdef _LINUX_
const std::string Server::readLine(const int socket) throw(ServerException) {
#endif
#ifdef _WIN_
const std::string Server::readLine(const int threadId, const SOCKET socket) const throw(ServerException) {
#endif
    auto result = std::string();

    char resolvedSymbol = ' ';

#ifdef _LINUX_
    for(auto index = 0; index < MESSAGE_SIZE; ++index) {
        if(readSize <= 0)
            throw ServerException(COULD_NOT_RECEIVE_MESSAGE);
        else if(resolvedSymbol == '\n')
            break;
        else if(resolvedSymbol != '\r')
            result.push_back(resolvedSymbol);
    }
#endif
#ifdef _WIN_
    while(!this->generalInterrupt) {
        auto userFind = this->users.find(threadId);
        if(userFind == this->users.end() || userFind->second->interrupt)
            throw ServerException(COULD_NOT_RECEIVE_MESSAGE);

        auto readSize = recv(socket, &resolvedSymbol, 1, EMPTY_FLAGS);
        if(readSize == 0)
            throw ServerException(COULD_NOT_RECEIVE_MESSAGE);
        else if(readSize < 0)
            continue;
        else if(result.size() > MESSAGE_SIZE || resolvedSymbol == '\n')
            break;
        else if(resolvedSymbol != '\r')
            result.push_back(resolvedSymbol);
    }
#endif

    return result;
}

#ifdef _LINUX_
const void Server::createClientThread(const int clientSocket, sockaddr_in* address) {
#endif
#ifdef _WIN_
const void Server::createClientThread(const SOCKET clientSocket, sockaddr_in* address) {
#endif
    bool lockUsers = this->mutexUsers.try_lock();

    int index = 0;
    std::map<int, Server::User*>::iterator result;
    do {
        result = this->users.find(index);
        ++index;
    } while(result != this->users.end());
    --index;

    auto bind = std::bind(&Server::clientThreadInitialize, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    auto user = new User;
    user->userName = "";
    user->thread = std::make_shared<std::thread>(bind, this, index, clientSocket);
    user->socket = clientSocket;
#ifdef _WIN_
    user->interrupt = false;
#endif
    user->address = address;

    this->users.insert(std::pair<int, Server::User*>(index, user));

    if(lockUsers)
        this->mutexUsers.unlock();
}

const void Server::removeClientThread(const int threadId) throw(ServerException) {
    bool lockUsers = this->mutexUsers.try_lock();

    const auto find = this->users.find(threadId);
    if(find != this->users.end()) {
        const auto clientSocket = find->second->socket;
        clearSocket(threadId, clientSocket);
        this->users.at(threadId)->thread.get()->detach();
        this->users.erase(threadId);
    }

    if(lockUsers)
        this->mutexUsers.unlock();
}

#ifdef _LINUX_
const void Server::clearSocket(const int threadId, const int socket) throw(ServerException) {
#endif
#ifdef _WIN_
const void Server::clearSocket(const int threadId, const SOCKET socket) throw(ServerException) {
#endif

#ifdef _LINUX_
    if(socket < 0)
        return;

    auto socketShutdown = shutdown(socket, SHUT_RDWR);
    if(socketShutdown != 0)
        throw ServerException(COULD_NOT_SHUT_SOCKET_DOWN);
#endif
#ifdef _WIN_
    if(socket == INVALID_SOCKET)
        return;

    if(threadId != -1) {
        auto socketShutdown = shutdown(socket, SD_BOTH);
        if(socketShutdown == SOCKET_ERROR)
            throw ServerException(COULD_NOT_SHUT_SOCKET_DOWN);
    }
#endif

    bool lockOut = this->mutexOut.try_lock();
    *this->out <<  "Thread 0x" << threadId << ". Socket is down." << std::endl;
    if(lockOut)
        this->mutexOut.unlock();

#ifdef _LINUX_
    auto socketClose = close(socket);
    if(socketClose != 0)
        throw ServerException(COULD_NOT_CLOSE_SOCKET);
#endif
#ifdef _WIN_
    auto socketClose = closesocket(socket);
    if(socketClose == SOCKET_ERROR)
        throw ServerException(COULD_NOT_CLOSE_SOCKET);
#endif

    lockOut = this->mutexOut.try_lock();
    *this->out <<  "Thread 0x" << threadId << ". Socket is closed." << std::endl;
    if(lockOut)
        this->mutexOut.unlock();
}

const void Server::lockAll(bool tryLockArray[9]) {
    tryLockArray[0] = this->mutexFilename.try_lock();
    tryLockArray[1] = this->mutexEvents.try_lock();
    tryLockArray[2] = this->mutexAccounts.try_lock();
    tryLockArray[3] = this->mutexTimings.try_lock();
    tryLockArray[4] = this->mutexUsers.try_lock();
    tryLockArray[5] = this->mutexSubscriptions.try_lock();
    tryLockArray[6] = this->mutexIn.try_lock();
    tryLockArray[7] = this->mutexOut.try_lock();
    tryLockArray[8] = this->mutexError.try_lock();
}
const void Server::unlockAll(const bool tryLockArray[9]) {
    if(tryLockArray[0]) this->mutexError.unlock();
    if(tryLockArray[1]) this->mutexOut.unlock();
    if(tryLockArray[2]) this->mutexIn.unlock();
    if(tryLockArray[3]) this->mutexSubscriptions.unlock();
    if(tryLockArray[4]) this->mutexUsers.unlock();
    if(tryLockArray[5]) this->mutexTimings.unlock();
    if(tryLockArray[6]) this->mutexAccounts.unlock();
    if(tryLockArray[7]) this->mutexEvents.unlock();
    if(tryLockArray[8]) this->mutexFilename.unlock();
}

const void Server::stop() throw(ServerException){
    this->generalInterrupt = true;
    this->timerInterrupt = true;

    bool tryLockArray[9];
    lockAll(tryLockArray);

#ifdef _LINUX_
    if(generalSocket >= 0) {
        auto fcntlResult = fcntl(generalSocket, F_SETFL, generalFlags);
        if (fcntlResult < 0) {
            unlockAll(tryLockArray);
            throw ServerException(COULD_NOT_SET_NON_BLOCKING);
        }
    }
#endif
#ifdef _WIN_
    if(generalSocket != INVALID_SOCKET) {
        unsigned int iMode = 0;
        auto ioctlResult = ioctlsocket(generalSocket, 0x8004667E, &iMode);
        if(ioctlResult == SOCKET_ERROR)
            throw ServerException(COULD_NOT_SET_NON_BLOCKING);
    }
#endif

    if(timerThread != nullptr && timerThread.get()->joinable())
        timerThread.get()->join();

    if(commandThread != nullptr && commandThread.get()->joinable())
        commandThread.get()->join();

    for (auto &current: this->users) {
#ifdef _LINUX_
        auto temp = current.second->socket;
        current.second->socket = -1;
        this->serverPtr->clearSocket(current.first, temp);

        if (current.second->thread != nullptr && current.second->thread.get()->joinable())
            current.second->thread.get()->join();
#endif
#ifdef _WIN_
        current.second->interrupt = true;
#endif
    }

    clearSocket(-1, generalSocket);

#ifdef _WIN_
    WSACleanup();
#endif

    unlockAll(tryLockArray);

    this->controller->finit(BACKUP_FILENAME);
    try { this->controller->save(); }
    catch (const ServerController::ControllerException& exception) {  }
}

Server::~Server() {
    stop();
}

// ServerController

Server::ServerController::ServerController(Server *serverPtr) {
    this->serverPtr = serverPtr;
}

#ifdef _LINUX_
const std::pair<std::string, std::string> Server::ServerController::getAddressInfoBySocket(const int socket) const throw(ControllerException) {
#endif
#ifdef _WIN_
const std::pair<std::string, std::string> Server::ServerController::getAddressInfoBySocket(const SOCKET socket) const throw(ControllerException) {
#endif

    if(socket == -1)
        throw ControllerException(COULD_NOT_GET_CLIENT_INFO_BY_SOCKET);

    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    for(auto& current: this->serverPtr->users)
        if(current.second->socket == socket) {
            auto result = std::pair<std::string, std::string>(std::string(inet_ntoa(current.second->address->sin_addr)), std::to_string(current.second->address->sin_port));
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            return result;
        }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();
    throw ControllerException(USER_IS_NOT_EXISTS);
}

#ifdef _LINUX_
const char* Server::ServerController::getUserNameBySocket(const int clientSocket) const throw(ControllerException) {
#endif
#ifdef _WIN_
const char* Server::ServerController::getUserNameBySocket(const SOCKET clientSocket) const throw(ControllerException) {
#endif

    if(clientSocket < 0)
        throw ControllerException(COULD_NOT_GET_CLIENT_INFO_BY_SOCKET);

    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    for(auto& current: this->serverPtr->users)
        if(current.second->socket == clientSocket) {
            if(current.second->userName.empty()) {
                if(lockUsers)
                    this->serverPtr->mutexUsers.unlock();
                throw ControllerException(USER_IS_NOT_CONNECTED_YET);
            }

            const char* result = current.second->userName.data();
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            return result;
        }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();
    throw ControllerException(USER_IS_NOT_EXISTS);
}
const int Server::ServerController::getThreadIdByUserName(const char* userName) const throw(ControllerException) {
    if(userName == nullptr || strlen(userName) == 0)
        throw ControllerException(USER_IS_NOT_CONNECTED_YET);

    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    auto string = std::string(userName);
    for(auto& current: this->serverPtr->users)
        if(current.second->userName == string) {
            const int result = current.first;
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            return result;
        }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();
    throw ControllerException(USER_IS_NOT_EXISTS);
}
const int Server::ServerController::getEventIdByEventName(const char *eventName) const throw(ControllerException) {
    auto string = std::string(eventName);

    bool lockEvents = this->serverPtr->mutexEvents.try_lock();

    for(auto& current: this->serverPtr->events)
        if(current.second->eventName == string) {
            const int result = current.first;
            if(lockEvents)
                this->serverPtr->mutexEvents.unlock();
            return result;
        }

    if(lockEvents)
        this->serverPtr->mutexEvents.unlock();
    throw ControllerException(EVENT_IS_NOT_EXISTS);
}
const char* Server::ServerController::getEventNameByEventId(const int eventId) const throw(ControllerException) {
    bool lockEvents = this->serverPtr->mutexEvents.try_lock();

    auto eventFind = this->serverPtr->events.find(eventId);
    if(eventFind == this->serverPtr->events.end()) {
        if(lockEvents)
            this->serverPtr->mutexEvents.unlock();
        throw ControllerException(EVENT_IS_NOT_EXISTS);
    }
    else {
        const char* result = this->serverPtr->events.at(eventId)->eventName.data();
        if(lockEvents)
            this->serverPtr->mutexEvents.unlock();
        return result;
    }
}
const char* Server::ServerController::getUserNameByThreadId(const int threadId) const throw(ControllerException) {
    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    auto userFind = this->serverPtr->users.find(threadId);
    if(userFind == this->serverPtr->users.end()) {
        if(lockUsers)
            this->serverPtr->mutexUsers.unlock();
        throw ControllerException(USER_IS_NOT_EXISTS);
    }
    else {
        if(this->serverPtr->users.at(threadId)->userName.empty()) {
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            throw ControllerException(USER_IS_NOT_CONNECTED_YET);
        }

        const char* result = this->serverPtr->users.at(threadId)->userName.data();
        if(lockUsers)
            this->serverPtr->mutexUsers.unlock();
        return result;
    }
}

const void Server::ServerController::eventCreate(const char *eventName, const std::chrono::milliseconds& start, const std::chrono::seconds& period) const throw(ControllerException) {
    try { getEventIdByEventName(eventName); }
    catch (const ControllerException& exception) {
        bool lockEvents = this->serverPtr->mutexEvents.try_lock();

        int index = 0;
        std::map<int, Server::Event*>::iterator result;
        do {
            result = this->serverPtr->events.find(index);
            ++index;
        } while(result != this->serverPtr->events.end());
        --index;

        auto event = new Event;
        event->eventName = std::string(eventName);
        event->startMoment = start;
        event->period = period;
        this->serverPtr->events.insert(std::pair<int, Server::Event*>(index, event));

        if(lockEvents)
            this->serverPtr->mutexEvents.unlock();

        this->serverPtr->refreshTiming(index);
        return;
    }

    throw ControllerException(EVENT_IS_ALREADY_EXISTS);
}
const void Server::ServerController::eventDrop(const char *eventName) const throw(ControllerException) {
    auto eventId = getEventIdByEventName(eventName);

    bool lockEvents = this->serverPtr->mutexEvents.try_lock();
    this->serverPtr->events.erase(eventId);
    if(lockEvents)
        this->serverPtr->mutexEvents.unlock();

    this->serverPtr->refreshTiming(eventId);
}
const void Server::ServerController::eventNotify(const char *eventName) const {
    bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();
    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    auto currentTime = time(0);
    struct tm* nowStruct = localtime(&currentTime);

    auto eventString = std::string(eventName);
    for(auto& currentSubscription: this->serverPtr->subscriptions)
        if(currentSubscription.second == eventString)
            for(auto& currentUsers : this->serverPtr->users)
                if(currentUsers.second->userName == currentSubscription.first) {
                    std::stringstream stream;
                    stream << nowStruct->tm_hour << ":" << nowStruct->tm_min << ":" << nowStruct->tm_sec << " Notify about the event \"" << currentSubscription.second << "\"." << std::endl;
                    writeLine(stream.str(), currentUsers.second->socket);
                }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();
    if(lockSubscriptions)
        this->serverPtr->mutexSubscriptions.unlock();
}
const void Server::ServerController::eventSubscribe(const char *eventName, const char *userName) const throw(ControllerException) {
    getEventIdByEventName(eventName);
    getThreadIdByUserName(userName);

    std::string eventString(eventName);
    std::string userString(userName);

    bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();

    auto position = this->serverPtr->subscriptions.end();

    for(auto current = this->serverPtr->subscriptions.begin(); current != this->serverPtr->subscriptions.end(); ++current) {
        if(current->first == userString) {
            if (current->second == eventString) {
                if(lockSubscriptions)
                    this->serverPtr->mutexSubscriptions.unlock();
                return;
            }

            position = current;
        }
    }

    this->serverPtr->subscriptions.insert(position, std::pair<std::string, std::string>(userString, eventString));
    if(lockSubscriptions)
        this->serverPtr->mutexSubscriptions.unlock();
}
const void Server::ServerController::eventUnsubscribe(const char *eventName, const char *userName) const throw(ControllerException) {
    getEventIdByEventName(eventName);
    getThreadIdByUserName(userName);

    std::string eventString(eventName);
    std::string userString(userName);

    bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();

    for(auto current = this->serverPtr->subscriptions.begin(); current != this->serverPtr->subscriptions.end(); ++current)
        if(current->first == userString && current->second == eventString) {
            this->serverPtr->subscriptions.erase(current);
            if(lockSubscriptions)
                this->serverPtr->mutexSubscriptions.unlock();
            return;
        }

    if(lockSubscriptions)
        this->serverPtr->mutexSubscriptions.unlock();
}

const void Server::ServerController::help(std::ostream* out) const {
    auto stream = (out == nullptr) ? this->serverPtr->out : out;
    const bool client = (out != nullptr);

    bool lockOut = false;

    if(!client)
        lockOut = this->serverPtr->mutexOut.try_lock();

    const int length = 77;

    if(client) {
        *stream << std::endl
                << std::left << std::setw(length) << std::setfill(' ') << "COMMAND"
                << std::left << std::setw(length) << std::setfill(' ') << "DESCRIPTION" << std::endl;

        *stream << std::endl << "Connect commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "connect <user[a-z][0-9]> <password[a-z][0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Connect to server by username and password." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "exit"
                << std::left << std::setw(length) << std::setfill(' ') << "Disconnect from server and remove session." << std::endl;

        *stream << std::endl << "Info commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info self"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about current connected user." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info events"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about all existing events." << std::endl;

        *stream << std::endl << "Account commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "register <user[a-z][0-9]> <password[a-z][0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Register new user in accounts table." << std::endl;

        *stream << std::endl << "Event commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event create single <event[a-z][0-9]> <[dd/mm/yyyy|hh:mm:ss]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Create single event." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event create multi <event[a-z][0-9]> <[dd/mm/yyyy|hh:mm:ss]> <period[int]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Create repeating event with period in seconds." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event drop <event[a-z][0-9] | #EID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Remove event by name or id." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event subscribe <event[a-z][0-9] | #EID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Sign current connected user on event." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event unsubscribe <event[a-z][0-9] | #EID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Unsubscribe current connected user from event." << std::endl << std::endl;
    }
    else {
        *stream << std::endl
                << std::left << std::setw(length) << std::setfill(' ') << "COMMAND"
                << std::left << std::setw(length) << std::setfill(' ') << "DESCRIPTION" << std::endl;

        *stream << std::endl << "Server state commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "filename <filename^[\\/:*?\"<>|]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Set current filename (default \"server.data\")." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "save"
                << std::left << std::setw(length) << std::setfill(' ') << "Save program data into current file." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "load"
                << std::left << std::setw(length) << std::setfill(' ') << "Load program data from current file." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "exit"
                << std::left << std::setw(length) << std::setfill(' ') << "Close all connections and save backup data (into file \"server.backup\")." << std::endl;

        *stream << std::endl << "Info commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info user <login[a-z][0-9] | #UID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about connected user." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info users"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about all connected users." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info accounts"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about all existing accounts." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info events"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about all existing events." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "info subscriptions"
                << std::left << std::setw(length) << std::setfill(' ') << "Print info about all existing subscriptions." << std::endl;

        *stream << std::endl << "Account commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "register <user[a-z][0-9]> <password[a-z][0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Register new user in accounts table." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "detach <user[a-z][0-9] | #UID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Disconnect user from server." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "delete <user[a-z][0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Detach user and delete him from accounts table." << std::endl;

        *stream << std::endl << "Event commands:" << std::endl << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event create single <event[a-z][0-9]> <[dd/mm/yyyy|hh:mm:ss]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Create single event." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event create multi <event[a-z][0-9]> <[dd/mm/yyyy|hh:mm:ss]> <period[int]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Create repeating event with period in seconds." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event drop <event[a-z][0-9] | #EID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Remove event by name or id." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event subscribe <event[a-z][0-9] | #EID[0-9]> <user[a-z][0-9] | #UID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Sign user on event." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event unsubscribe <event[a-z][0-9] | #EID[0-9]> <user[a-z][0-9] | #UID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Unsubscribe user from event." << std::endl;

        *stream << std::left << std::setw(length) << std::setfill(' ') << "event notify <event[a-z][0-9] | #EID[0-9]>"
                << std::left << std::setw(length) << std::setfill(' ') << "Notify all subscribers about the event immediately." << std::endl << std::endl;
    }

    if(!client && lockOut)
        this->serverPtr->mutexOut.unlock();
}

const void Server::ServerController::printSelfInfo(std::ostream* out, const char* userName) const {
    if(userName == nullptr || strlen(userName) == 0)
        throw ControllerException(USER_IS_NOT_CONNECTED_YET);

    auto stream = (out == nullptr) ? this->serverPtr->out : out;

    bool lockOut = false;

    if(out == nullptr)
        lockOut = this->serverPtr->mutexOut.try_lock();

    bool lockUsers = this->serverPtr->mutexUsers.try_lock();
    bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();

    auto userString = std::string(userName);
    for (auto &currentUser: this->serverPtr->users)
        if(currentUser.second->userName == userString) {
            *stream << std::endl
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "ID"
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "USERNAME"
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "IP"
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "PORT"
                    << std::endl << std::endl;

            std::pair<std::string, std::string> addressInfo;
            try { addressInfo = getAddressInfoBySocket(currentUser.second->socket); }
            catch (...) { addressInfo = std::pair<std::string, std::string>("", ""); }

            *stream << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << currentUser.first
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((!currentUser.second->userName.empty()) ? currentUser.second->userName : "Not connected yet.")
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((!addressInfo.first.empty()) ? addressInfo.first : "Couldn't resolve IP.")
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((!addressInfo.second.empty()) ? addressInfo.second : "Couldn't resolve PORT.")
                    << std::endl << std::endl;

            bool first = true;
            for(auto &currentSubscribe: this->serverPtr->subscriptions) {
                if(currentSubscribe.first != currentUser.second->userName)
                    continue;

                if(first) {
                    first = false;

                    *stream << std::endl
                            << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "EVENTNAME"
                            << std::endl<< std::endl;
                }

                *stream << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << currentSubscribe.second
                        << std::endl;
            }

            if(first)
                *stream << "User has no subscriptions." << std::endl;

            *stream << std::endl;

            if(lockSubscriptions)
                this->serverPtr->mutexSubscriptions.unlock();
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();

            if(out == nullptr && lockOut)
                this->serverPtr->mutexOut.unlock();

            return;
        }

    if(lockSubscriptions)
        this->serverPtr->mutexSubscriptions.unlock();
    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();

    if(out == nullptr && lockOut)
        this->serverPtr->mutexOut.unlock();

    throw ControllerException(USER_IS_NOT_EXISTS);
}
const void Server::ServerController::printSubscriptionsInfo() const {
    bool lockOut = this->serverPtr->mutexOut.try_lock();
    bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();

    if(this->serverPtr->subscriptions.empty())
        *this->serverPtr->out << "Subscriptions table is empty." << std::endl;
    else {
        *this->serverPtr->out << std::endl
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "USERNAME"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "EVENTNAME"
                              << std::endl;

        std::string userName;

        for (auto &current: this->serverPtr->subscriptions) {
            *this->serverPtr->out << ((userName != current.first) ? "\n" : "")
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((userName != current.first) ? current.first : "")
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.second
                                  << std::endl;

            userName = current.first;
        }

        *this->serverPtr->out << std::endl;
    }

    if(lockSubscriptions)
        this->serverPtr->mutexSubscriptions.unlock();
    if(lockOut)
        this->serverPtr->mutexOut.unlock();
}
const void Server::ServerController::printEventsInfo(std::ostream* out) const {
    auto stream = (out == nullptr) ? this->serverPtr->out : out;

    bool lockOut = false;

    if(out == nullptr)
        lockOut = this->serverPtr->mutexOut.try_lock();

    bool lockEvents = this->serverPtr->mutexEvents.try_lock();

    if(this->serverPtr->events.empty())
        *stream << "Events table is empty." << std::endl;
    else {
        *stream << std::endl
                << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "ID"
                << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "EVENTNAME"
                << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "START"
                << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "PERIOD"
                << std::endl << std::endl;

        for (auto &current: this->serverPtr->events) {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> castPoint(current.second->startMoment);
            auto time = std::chrono::system_clock::to_time_t(castPoint);
            std::string timeString = std::ctime(&time);
            timeString.resize(timeString.size() - 1);

            __int64_t days, hours, minutes, seconds;
            __int64_t period = current.second->period.count();
            std::string result;
            if(period != 0) {
                days = period / (60 * 60 * 24);
                hours = (period % (60 * 60 * 24)) / (60 * 60);
                minutes = (period % (60 * 60)) / 60;
                seconds = period % 60;
                result = std::to_string(days) + " days " + std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
            }

            *stream << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << (std::string(1, Command::PREFIX) + std::to_string(current.first))
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.second->eventName
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << timeString
                    << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((period == 0) ? "Single event." : result)
                    << std::endl;
        }

        *stream << std::endl;
    }

    if(lockEvents)
        this->serverPtr->mutexEvents.unlock();

    if(out == nullptr && lockOut)
        this->serverPtr->mutexOut.unlock();
}
const void Server::ServerController::printUsersInfo() const {
    bool lockOut = this->serverPtr->mutexOut.try_lock();
    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    if(this->serverPtr->users.empty())
        *this->serverPtr->out << "Users table is empty." << std::endl;
    else {
        *this->serverPtr->out << std::endl
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "ID"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "USERNAME"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "IP"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "PORT"
                              << std::endl << std::endl;

        for (auto &current: this->serverPtr->users) {
            std::pair<std::string, std::string> addressInfo;
            try { addressInfo = getAddressInfoBySocket(current.second->socket); }
            catch (...) { addressInfo = std::pair<std::string, std::string>("", ""); }

            *this->serverPtr->out << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << (std::string(1, Command::PREFIX) + std::to_string(current.first))
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((!current.second->userName.empty()) ? current.second->userName : "Not connected yet.")
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((!addressInfo.first.empty()) ? addressInfo.first : "Couldn't resolve IP.")
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((!addressInfo.second.empty()) ? addressInfo.second : "Couldn't resolve PORT.")
                                  << std::endl;
        }

        *this->serverPtr->out << std::endl;
    }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();
    if(lockOut)
        this->serverPtr->mutexOut.unlock();
}
const void Server::ServerController::printAccountsInfo() const {
    bool lockOut = this->serverPtr->mutexOut.try_lock();
    bool lockAccounts = this->serverPtr->mutexAccounts.try_lock();

    if(this->serverPtr->accounts.empty())
        *this->serverPtr->out << "Accounts table is empty." << std::endl;
    else {
        *this->serverPtr->out << std::endl
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "USERNAME"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "PASSWORD"
                              << std::endl << std::endl;

        for (auto &current: this->serverPtr->accounts) {
            *this->serverPtr->out << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.first
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.second
                                  << std::endl;
        }
        *this->serverPtr->out << std::endl;
    }

    if(lockAccounts)
        this->serverPtr->mutexAccounts.unlock();
    if(lockOut)
        this->serverPtr->mutexOut.unlock();
}

const void Server::ServerController::exit() const {
    this->serverPtr->generalInterrupt = true;
}
const void Server::ServerController::finit(const char *filename) const {
    bool lockFilename = this->serverPtr->mutexFilename.try_lock();
    this->serverPtr->filename = std::string(filename);
    if(lockFilename)
        this->serverPtr->mutexFilename.unlock();
}
const void Server::ServerController::save() const throw(ControllerException) {
    bool lockFilename = this->serverPtr->mutexFilename.try_lock();
    auto filename = (this->serverPtr->filename.empty()) ? DEFAULT_FILENAME: this->serverPtr->filename;
    if(lockFilename)
        this->serverPtr->mutexFilename.unlock();

    auto stream = std::ofstream(filename);

    if(!stream.is_open())
        throw ControllerException(COULD_NOT_OPEN_FILE);

    bool lockEvents = this->serverPtr->mutexEvents.try_lock();
    bool lockAccounts = this->serverPtr->mutexAccounts.try_lock();
    bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();

    stream << this->serverPtr->events.size() << std::endl;
    for(auto& current: this->serverPtr->events)
        stream << current.second->eventName << std::endl << current.second->startMoment.count() << std::endl << current.second->period.count() << std::endl;

    stream << this->serverPtr->accounts.size() << std::endl;
    for(auto& currentAccount: this->serverPtr->accounts) {
        stream << currentAccount.first << std::endl << currentAccount.second << std::endl;

        int count = 0;
        for(auto& currentSubscribe: this->serverPtr->subscriptions)
            if(currentSubscribe.first == currentAccount.first)
                ++count;

        stream << count << std::endl;

        for(auto& currentSubscribe: this->serverPtr->subscriptions)
            if(currentSubscribe.first == currentAccount.first)
                stream << currentSubscribe.second << std::endl;
    }

    if(lockSubscriptions)
        this->serverPtr->mutexSubscriptions.unlock();
    if(lockAccounts)
        this->serverPtr->mutexAccounts.unlock();
    if(lockEvents)
        this->serverPtr->mutexEvents.unlock();

    stream.close();
}
const void Server::ServerController::load() const throw(ControllerException) {
    bool lockFilename = this->serverPtr->mutexFilename.try_lock();
    auto filename = (this->serverPtr->filename.empty()) ? DEFAULT_FILENAME: this->serverPtr->filename;
    if(lockFilename)
        this->serverPtr->mutexFilename.unlock();

    auto stream = std::ifstream(filename);
    if(!stream.is_open())
        throw ControllerException(COULD_NOT_OPEN_FILE);

    bool tryLockArray[9];
    this->serverPtr->lockAll(tryLockArray);

    if(this->serverPtr->timerThread != nullptr) {
        this->serverPtr->timerInterrupt = true;
        if(this->serverPtr->timerThread.get()->joinable())
            this->serverPtr->timerThread.get()->join();
    }

    this->serverPtr->timings.clear();
    this->serverPtr->events.clear();

    for(auto& current: this->serverPtr->users) {
#ifdef _LINUX_
        auto temp = current.second->socket;
        current.second->socket = -1;
        this->serverPtr->clearSocket(current.first, temp);

       if(current.second->thread != nullptr && current.second->thread.get()->joinable())
            current.second->thread.get()->join();
#endif
#ifdef _WIN_
        current.second->interrupt = true;
#endif
    }

    this->serverPtr->users.clear();

    std::stringstream stringStream;

    std::string eventName;
    std::string startString, periodString;
    __int64_t start, period;

    std::string countString;
    int count = -1, index = 0;

    while(!stream.eof()) {
        if(count == -1) {
            std::getline(stream, countString);
            if(countString.back() == '\r') countString.pop_back();
            stringStream.clear();
            stringStream.str(countString);
            stringStream >> count;
        }
        else if(count > 0) {
            std::getline(stream, eventName);
            if(eventName.back() == '\r') eventName.pop_back();
            if(stream.eof()) {
                this->serverPtr->unlockAll(tryLockArray);
                throw ControllerException(COULD_NOT_PARSE_FILE);
            }
            std::getline(stream, startString);
            if(startString.back() == '\r') startString.pop_back();
            stringStream.clear();
            stringStream.str(startString);
            stringStream >> start;
            if(stream.eof()) {
                this->serverPtr->unlockAll(tryLockArray);
                throw ControllerException(COULD_NOT_PARSE_FILE);
            }
            std::getline(stream, periodString);
            if(periodString.back() == '\r') periodString.pop_back();
            stringStream.clear();
            stringStream.str(periodString);
            stringStream >> period;

            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            auto buffer = Command::EVENT_BUFFER;
            if(((period != 0) || (now + std::chrono::seconds(buffer) < std::chrono::milliseconds(start))) && Command::MAX_EVENT_PERIOD >= period && (period == 0 || Command::MIN_EVENT_PERIOD <= period)) {
                try {
                    auto event = new Event;
                    event->eventName = Command::checkASCII(eventName);
                    event->startMoment = std::chrono::milliseconds(start);
                    event->period = std::chrono::seconds(period);
                    this->serverPtr->events.insert(std::pair<int, Server::Event*>(index, event));
                    ++index;
                } catch (const Command::CommandException &exception) {
                    this->serverPtr->unlockAll(tryLockArray);
                    throw ControllerException(COULD_NOT_PARSE_FILE);
                }
            }

            --count;
        }
        else
            break;
    }

    count = -1;

    std::string login, password;
    int subCount;

    while(!stream.eof()) {
        if(count == -1) {
            std::getline(stream, countString);
            if(countString.back() == '\r') countString.pop_back();
            stringStream.clear();
            stringStream.str(countString);
            stringStream >> count;
        }
        else if(count > 0) {
            std::getline(stream, login);
            if(login.back() == '\r') login.pop_back();
            if(stream.eof()) {
                this->serverPtr->unlockAll(tryLockArray);
                throw ControllerException(COULD_NOT_PARSE_FILE);
            }
            std::getline(stream, password);
            if(password.back() == '\r') password.pop_back();
            if(stream.eof()) {
                this->serverPtr->unlockAll(tryLockArray);
                throw ControllerException(COULD_NOT_PARSE_FILE);
            }
            std::getline(stream, countString);
            if(countString.back() == '\r') countString.pop_back();
            stringStream.clear();
            stringStream.str(countString);
            stringStream >> subCount;

            std::vector<std::string> resultSubs;
            std::string currentSub;
            for(auto subIndex = 0; subIndex < subCount; ++subIndex) {
                if(stream.eof()) {
                    this->serverPtr->unlockAll(tryLockArray);
                    throw ControllerException(COULD_NOT_PARSE_FILE);
                }
                std::getline(stream, currentSub);
                if(currentSub.back() == '\r') currentSub.pop_back();

                bool containsInCurrent = false;
                for(auto& current: resultSubs)
                    if(currentSub == current) {
                        containsInCurrent = true;
                        break;
                    }

                bool containsInEvents = false;
                for(auto& current: this->serverPtr->events)
                    if(currentSub == current.second->eventName) {
                        containsInEvents = true;
                        break;
                    }

                if(!containsInCurrent && containsInEvents)
                    resultSubs.push_back(currentSub);
            }

            try {
                this->serverPtr->accounts.insert(std::pair<std::string, std::string>(Command::checkASCII(login), Command::checkASCII(password)));

                for(auto& current: resultSubs)
                    this->serverPtr->subscriptions.push_back(std::pair<std::string, std::string>(Command::checkASCII(login), Command::checkASCII(current)));
            } catch (const Command::CommandException& exception) {
                this->serverPtr->unlockAll(tryLockArray);
                throw ControllerException(COULD_NOT_PARSE_FILE);
            }
            --count;
        }
        else
            break;
    }

    stream.close();

    for(auto& current: this->serverPtr->events)
        this->serverPtr->refreshTiming(current.first);

    this->serverPtr->timerInterrupt = false;
    auto bind = std::bind(&Server::timerThreadInitialize, std::placeholders::_1);
    this->serverPtr->timerThread = std::make_shared<std::thread>(bind, this->serverPtr);

    this->serverPtr->unlockAll(tryLockArray);
    return;
}
const void Server::ServerController::detach(const char *userName) const throw(ControllerException) {
    if(userName == nullptr || strlen(userName) == 0)
        throw ControllerException(USER_IS_NOT_CONNECTED_YET);

    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    for(auto& current: this->serverPtr->users)
        if(current.second->userName == userName) {
#ifdef _LINUX_
            auto temp = current.second->socket;
            current.second->socket = -1;
            this->serverPtr->clearSocket(current.first, temp);

            if(current.second->thread != nullptr && current.second->thread.get()->joinable())
                current.second->thread.get()->join();
#endif
#ifdef _WIN_
            current.second->interrupt = true;
#endif

            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            return;
        }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();
    throw ControllerException(USER_IS_NOT_EXISTS);
}

#ifdef _LINUX_
const void Server::ServerController::close(const int socket) const throw(ControllerException) {
#endif
#ifdef _WIN_
const void Server::ServerController::close(const SOCKET socket) const throw(ControllerException) {
#endif

    bool lockUsers = this->serverPtr->mutexUsers.try_lock();

    for(auto& current: this->serverPtr->users)
        if(current.second->socket == socket) {
#ifdef _LINUX_
            auto temp = current.second->socket;
            current.second->socket = -1;
            this->serverPtr->clearSocket(current.first, temp);
#endif
#ifdef _WIN_
            current.second->interrupt = true;
#endif

            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();

            return;
        }

    if(lockUsers)
        this->serverPtr->mutexUsers.unlock();

    throw ControllerException(USER_IS_NOT_EXISTS);
}

const void Server::ServerController::del(const char *userName) const throw(ControllerException) {
    if(userName == nullptr || strlen(userName) == 0)
        throw ControllerException(USER_IS_NOT_CONNECTED_YET);

    bool lockAccounts = this->serverPtr->mutexAccounts.try_lock();

    auto userFind = this->serverPtr->accounts.find(std::string(userName));
    if(userFind == this->serverPtr->accounts.end()) {
        if(lockAccounts)
            this->serverPtr->mutexAccounts.unlock();
        throw ControllerException(USER_IS_NOT_EXISTS);
    }
    else {
        try { detach(userName); }
        catch (const ControllerException& exception) { }

        bool lockSubscriptions = this->serverPtr->mutexSubscriptions.try_lock();

        for(auto current = this->serverPtr->subscriptions.begin(); current != this->serverPtr->subscriptions.end(); ++current)
            if(current->first == std::string(userName)) {
                this->serverPtr->subscriptions.erase(current);
                --current;
            }

        if(lockSubscriptions)
            this->serverPtr->mutexSubscriptions.unlock();

        this->serverPtr->accounts.erase(userFind);
        if(lockAccounts)
            this->serverPtr->mutexAccounts.unlock();
    }
}
const void Server::ServerController::reg(const char *userName, const char *password) const throw(ControllerException) {
    if(userName == nullptr || strlen(userName) == 0)
        throw ControllerException(USER_IS_NOT_CONNECTED_YET);

    bool lockAccounts = this->serverPtr->mutexAccounts.try_lock();

    auto userFind = this->serverPtr->accounts.find(userName);
    if(userFind == this->serverPtr->accounts.end()) {
        this->serverPtr->accounts.insert(std::pair<std::string, std::string>(userName, password));
        if(lockAccounts)
            this->serverPtr->mutexAccounts.unlock();
    }
    else {
        if(lockAccounts)
            this->serverPtr->mutexAccounts.unlock();
        throw ControllerException(USER_IS_ALREADY_EXISTS);
    }
}

#ifdef _LINUX_
const void Server::ServerController::connect(const char* userName, const char* password, const int clientSocket) const throw(ControllerException) {
#endif
#ifdef _WIN_
const void Server::ServerController::connect(const char* userName, const char* password, const SOCKET clientSocket) const throw(ControllerException) {
#endif

    try { getThreadIdByUserName(userName); }
    catch (const ControllerException& exception) {
        bool lockUsers = this->serverPtr->mutexUsers.try_lock();

        std::map<int, User*>::iterator userFind;
        for(userFind = this->serverPtr->users.begin(); userFind != this->serverPtr->users.end(); ++userFind)
            if(userFind->second->socket == clientSocket) {
                if(!userFind->second->userName.empty()) {
                    if(lockUsers)
                        this->serverPtr->mutexUsers.unlock();
                    throw ControllerException(USER_IS_ALREADY_CONNECTED);
                }
                break;
            }

        bool lockAccounts = this->serverPtr->mutexAccounts.try_lock();

        auto accountFind = this->serverPtr->accounts.find(userName);
        if(userFind == this->serverPtr->users.end() || accountFind == this->serverPtr->accounts.end()) {
            if(lockAccounts)
                this->serverPtr->mutexAccounts.unlock();
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            throw ControllerException(USER_IS_NOT_EXISTS);
        }

        if(lockAccounts)
            this->serverPtr->mutexAccounts.unlock();

        if(accountFind->second != std::string(password)) {
            if(lockUsers)
                this->serverPtr->mutexUsers.unlock();
            throw ControllerException(WRONG_PASSWORD);
        }

        userFind->second->userName = userName;

        if(lockUsers)
            this->serverPtr->mutexUsers.unlock();

        return;
    }

    throw ControllerException(USER_IS_ALREADY_CONNECTED);
}

// ServerController::ControllerException

Server::ServerController::ControllerException::ControllerException(const Server::ServerController::Error error) {
    this->error = error;
}

const char* Server::ServerController::ControllerException::what() const noexcept {
    switch(this->error) {
        case COULD_NOT_OPEN_FILE:
            return "It's impossible to open file.";

        case COULD_NOT_PARSE_FILE:
            return "It's impossible to parse file.";

        case USER_IS_ALREADY_EXISTS:
            return "User is already exists.";

        case USER_IS_NOT_EXISTS:
            return "User isn't exists.";

        case USER_IS_NOT_CONNECTED_YET:
            return "User isn't connected yet.";

        case USER_IS_ALREADY_CONNECTED:
            return "User is already connected.";

        case EVENT_IS_ALREADY_EXISTS:
            return "Event is already exists.";

        case EVENT_IS_NOT_EXISTS:
            return "Event isn't exists.";

        case COULD_NOT_GET_CLIENT_INFO_BY_SOCKET:
            return "It's impossible to get info by socket.";

        case WRONG_PASSWORD:
            return "Wrong password.";
    }

    return "Unknown exception.";
}

const int Server::ServerController::ControllerException::code() const {
    return this->error;
}

// ServerException

Server::ServerException::ServerException(const Server::Error error) {
    this->error = error;
}

const char* Server::ServerException::what() const noexcept {
    switch(this->error){
        case COULD_NOT_CREATE_SOCKET:
            return "It's impossible to create a socket.";

        case COULD_NOT_BIND:
            return "It's impossible to bind (Maybe server is already started on this port).";

        case COULD_NOT_SET_NON_BLOCKING:
            return "It's impossible to set non-blocking flag.";

        case COULD_NOT_ACCEPT:
            return "It's impossible to accept incoming message.";

        case COULD_NOT_RECEIVE_MESSAGE:
            return "It's impossible to receive message.";

        case COULD_NOT_SHUT_SOCKET_DOWN:
            return "It's impossible to shut socket down.";

        case COULD_NOT_CLOSE_SOCKET:
            return "It's impossible to close socket.";

#ifdef _WIN_
        case COULD_NOT_STARTUP:
            return "It's impossible to startup WSA.";

        case COULD_NOT_RESOLVE_ADDRESS:
            return "It's impossible to resolve address.";
#endif
    }

    return "Unknown exception.";
}

const int Server::ServerException::code() const {
    return this->error;
}
