#include "../headers/Server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <cstring>
#include <unistd.h>

Server::Server(std::ostream* out, std::ostream* error, const uint16_t port) throw(ServerException) {
    this->out = out;
    this->error = error;

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

    listen(generalSocket, BACKLOG);

    *this->out << "Listen socket." << std::endl;
}

const void Server::start() throw(ServerException){
    this->threads = std::map<int, std::shared_ptr<std::thread>>();

    while(true) {
        sockaddr_in clientAddress;
        auto size = sizeof(struct sockaddr_in);
        auto clientAccept = accept(generalSocket, (sockaddr *) &clientAddress, (socklen_t*) &size);
        if(clientAccept >= 0)
            createThread(clientAccept, clientAddress.sin_addr);
    }
}

void* Server::threadInitialize(void* thisPtr, const int threadId, const int clientAccept, const struct in_addr address) {
    try {
        ((Server*)thisPtr)->acceptClient(threadId, clientAccept, address);
    }
    catch (const ServerException& exception) {
        *((Server*)thisPtr)->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
    }
    catch(...) { }

    *((Server*)thisPtr)->out << "Thread 0x" << threadId << ". Client disconnected." << std::endl;
    ((Server*)thisPtr)->removeThread(threadId);
    return NULL;
}

const void Server::acceptClient(const int threadId, const int clientAccept, const struct in_addr address) throw(ServerException){
    *this->out << "Thread 0x" << threadId << ". Client with IP address " << inet_ntoa(address) << " has been connected." << std::endl;

    write(clientAccept, CLIENT_JOINED, strlen(CLIENT_JOINED));

    char message[MESSAGE_SIZE];
    bzero(message, sizeof message);

    while(true) {
        auto readSize = recv(clientAccept, message, MESSAGE_SIZE, EMPTY_FLAGS);

        if(readSize < 0)
            throw ServerException(Error::COULD_NOT_RECEIVE_MESSAGE);
        else if(readSize == 0)
            break;

        *this->out << "Thread 0x" << threadId << ". Received message: " << message << std::endl;

        write(clientAccept, message, strlen(message));
        bzero(message, sizeof message);
    }
}

const void Server::createThread(const int clientAccept, const struct in_addr address) {
    int index = 0;
    std::map<int, std::shared_ptr<std::thread>>::iterator result;
    do {
        result = this->threads.find(index);
        ++index;
    } while(result != this->threads.end());

    --index;

    auto bind = std::bind(&Server::threadInitialize, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    this->threads.insert(std::pair<int, std::shared_ptr<std::thread>>(index, std::make_shared<std::thread>(bind, this, index, clientAccept, address)));
    this->threads.at(index).get()->detach();
}

const void Server::removeThread(const int threadId) {
    this->threads.erase(threadId);
}

const void Server::stop() throw(ServerException){
    if(generalSocket < 0)
        return;

    auto socketShutdown = shutdown(generalSocket, SHUT_RDWR);
    if(socketShutdown != 0)
        throw ServerException(Error::COULD_NOT_SHUT_SOCKET_DOWN);

    *this->out << "Socket is down." << std::endl;

    auto socketClose = close(generalSocket);
    if(socketClose != 0)
        throw ServerException(Error::COULD_NOT_CLOSE_SOCKET);

    *this->out << "Socket is closed." << std::endl;
}

Server::~Server() {
    stop();
}

Server::ServerException::ServerException(const Server::Error error) {
    this->error = error;
}

const char* Server::ServerException::what() const noexcept {
    switch(this->error){
        case COULD_NOT_CREATE_SOCKET:
            return "It's impossible to create a socket.";

        case COULD_NOT_BIND:
            return "It's impossible to bind (Maybe server is already started on this port).";

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
    return error;
}
