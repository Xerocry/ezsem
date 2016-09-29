#include "../headers/Server.h"
#include "../headers/ServerCommand.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

Server::Server(std::ostream* out, std::istream* in, std::ostream* error, const uint16_t port) throw(ServerException) {
    this->out = out;
    this->in = in;
    this->error = error;

    this->clearStarted = false;

    generalSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(generalSocket < 0)
        throw ServerException(Error::COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    generalBind = bind(generalSocket, (sockaddr *) &serverAddress, sizeof(serverAddress));
    if(generalBind < 0)
        throw ServerException(Error::COULD_NOT_BIND);

    *this->out << "Socket has been successfully bind." << std::endl;

    generalFlags = fcntl(generalSocket, F_GETFL, 0);
    auto fcntlResult = fcntl (generalSocket, F_SETFL, generalFlags | O_NONBLOCK);
    if (fcntlResult < 0)
        throw ServerException(Error::COULD_NOT_SET_NON_BLOCKING);

    listen(generalSocket, BACKLOG);

    *this->out << "Listen socket." << std::endl;
}

const void Server::start() throw(ServerException){
    this->clientThreads = std::map<int, std::shared_ptr<std::thread>>();
    this->clientSockets = std::map<int, int>();

    this->interrupt = false;

    this->controller = new ServerController(this);

    auto bind = std::bind(&Server::commandThreadInitialize, std::placeholders::_1);
    commandThread = std::make_shared<std::thread>(bind, this);

    while(!this->interrupt) {
        sockaddr_in clientAddress;
        auto size = sizeof(struct sockaddr_in);
        auto clientSocket = accept(generalSocket, (sockaddr *) &clientAddress, (socklen_t *) &size);
        if (clientSocket >= 0)
            createClientThread(clientSocket, clientAddress.sin_addr);
    }
}

void* Server::commandThreadInitialize(void *thisPtr) {
    ((Server*)thisPtr)->commandExecutor();
    return NULL;
}

const void Server::commandExecutor() {
    std::string command;
    while(!this->interrupt){
        std::getline(*this->in, command);

        try { ServerCommand(&command, controller).parseAndExecute(); }
        catch(const Command::CommandException& exception) {
            *this->error << exception.what() << std::endl;
        }
        catch(const std::exception& exception) {
            *this->error << exception.what() << std::endl;
        }
        catch(...) {
            *this->error << "Strange resolve command error." << std::endl;
        }
    }
}

void* Server::clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket, const struct in_addr address) {
    auto serverPtr = ((Server*)thisPtr);

    try { serverPtr->acceptClient(threadId, clientSocket, address); }
    catch (const ServerException& exception) {
        *serverPtr->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
    }

    try {
        if(!serverPtr->clearStarted) {
            serverPtr->removeClientThread(threadId);
            *serverPtr->out << "Thread 0x" << threadId << ". Client disconnected." << std::endl;
        }
    } catch (const ServerException& exception) {
        *serverPtr->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
    }

    return NULL;
}

const void Server::acceptClient(const int threadId, const int clientSocket, const struct in_addr address) throw(ServerException){
    *this->out << "Thread 0x" << threadId << ". Client with IP address " << inet_ntoa(address) << " has been connected." << std::endl;

    write(clientSocket, CommunicationConstants::CLIENT_JOINED, strlen(CommunicationConstants::CLIENT_JOINED));

    char message[MESSAGE_SIZE];
    bzero(message, sizeof message);

    while(true) {
        auto readSize = recv(clientSocket, message, MESSAGE_SIZE, EMPTY_FLAGS);

        if(readSize < 0)
            throw ServerException(Error::COULD_NOT_RECEIVE_MESSAGE);
        else if(readSize == 0)
            break;

        *this->out << "Thread 0x" << threadId << ". Received message: " << message << std::endl;

        write(clientSocket, message, strlen(message));
        bzero(message, sizeof message);
    }
}

const void Server::createClientThread(const int clientSocket, const struct in_addr address) {
    int index = 0;
    std::map<int, std::shared_ptr<std::thread>>::iterator result;
    do {
        result = this->clientThreads.find(index);
        ++index;
    } while(result != this->clientThreads.end());

    --index;

    auto bind = std::bind(&Server::clientThreadInitialize, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4);
    this->clientThreads.insert(std::pair<int, std::shared_ptr<std::thread>>(index, std::make_shared<std::thread>(bind, this, index, clientSocket, address)));

    this->clientSockets.insert(std::pair<int, int>(index, clientSocket));
}

const void Server::removeClientThread(const int threadId) throw(ServerException) {
    const auto clientFind = this->clientSockets.find(threadId);
    if(clientFind != this->clientSockets.end()) {
        const auto clientSocket = clientFind->second;
        this->clientSockets.erase(clientFind);
        clearSocket(threadId, clientSocket);
    }

    const auto threadFind = this->clientThreads.find(threadId);
    if(threadFind != this->clientThreads.end()) {
        this->clientThreads.at(threadId).get()->detach();
        this->clientThreads.erase(threadId);
    }
}

const void Server::clearSocket(const int threadId, const int socket) throw(ServerException) {
    if(socket < 0)
        return;

    auto socketShutdown = shutdown(socket, SHUT_RDWR);
    if(socketShutdown != 0)
        throw ServerException(Error::COULD_NOT_SHUT_SOCKET_DOWN);

    *this->out <<  "Thread 0x" << threadId << ". Socket is down." << std::endl;

    auto socketClose = close(socket);
    if(socketClose != 0)
        throw ServerException(Error::COULD_NOT_CLOSE_SOCKET);

    *this->out <<  "Thread 0x" << threadId << ". Socket is closed." << std::endl;
}

const void Server::stop() throw(ServerException){
    this->clearStarted = true;

    auto fcntlResult = fcntl (generalSocket, F_SETFL, generalFlags);
    if (fcntlResult < 0)
        throw ServerException(Error::COULD_NOT_SET_NON_BLOCKING);

    if(commandThread.get()->joinable())
        commandThread.get()->join();
    for(auto& current: this->clientSockets) {
        clearSocket(current.first, current.second);
        *this->out << "Thread 0x" << current.first << ". Client disconnected." << std::endl;
    }

    this->clientSockets.clear();

    for(auto& current: this->clientThreads)
        if(current.second.get()->joinable())
            current.second.get()->join();

    this->clientThreads.clear();

    clearSocket(-1, generalSocket);
}

Server::~Server() {
    stop();
}

// ServerController

Server::ServerController::ServerController(Server *serverPtr) {
    this->serverPtr = serverPtr;
}

const void Server::ServerController::exit() const {
    this->serverPtr->interrupt = true;
}

const char* Server::ServerController::getEventNameByEventId(const int eventId) const { return nullptr; }
const char* Server::ServerController::getLoginByThreadId(const int threadId) const { return nullptr; }

const void Server::ServerController::eventDrop(const char *eventName) const { }

const void Server::ServerController::eventNotify(const char *eventName) const { }

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
    }

    return "Unknown exception.";
}

const int Server::ServerException::code() const {
    return this->error;
}
