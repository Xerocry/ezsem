#include <algorithm>
#include <string.h>
#include "../headers/Client.h"


#ifdef _LINUX_
Client::Client(std::ostream* out, std::istream* in, const uint16_t port, const char* address) throw(ClientException) {
#endif
#ifdef _WIN_
Client::Client(std::ostream* out, std::istream* in, const char* port, const char* address) throw(ClientException) {
#endif
    this->in = in;
    this->out = out;

#ifdef _LINUX_
    generalSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(generalSocket < 0)
        throw ClientException(COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

    struct sockaddr_in hints;
    bzero(&hints, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_port = htons(port);
    hints.sin_addr.s_addr = inet_addr(address);

    auto serverConnection = connect(generalSocket, (struct sockaddr *) &hints, sizeof(hints));
    if(serverConnection < 0)
        throw ClientException(COULD_NOT_CREATE_CONNECTION);

    *this->out << "Connection established." << std::endl;
#endif
#ifdef _WIN_
    WSADATA wsaData;
    generalWSAStartup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(generalWSAStartup != 0)
        throw ClientException(COULD_NOT_STARTUP);

    *this->out << "WSA has been successfully started." << std::endl;

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *addressResult = nullptr;
    auto wsaAddress = getaddrinfo(address, port, &hints, &addressResult);
    if (wsaAddress != 0)
        throw ClientException(COULD_NOT_RESOLVE_ADDRESS);

    *this->out << "Address and port have been successfully resolved." << std::endl;

    for(auto currentPtr = addressResult; currentPtr != nullptr; currentPtr = currentPtr->ai_next){
        generalSocket = socket(currentPtr->ai_family, currentPtr->ai_socktype, currentPtr->ai_protocol);
        if(generalSocket == INVALID_SOCKET)
            throw ClientException(COULD_NOT_CREATE_SOCKET);

        auto serverConnection = connect(generalSocket, currentPtr->ai_addr, (int) currentPtr->ai_addrlen);
        if(serverConnection != SOCKET_ERROR)
            break;

        closesocket(generalSocket);
        generalSocket = INVALID_SOCKET;
    }

    freeaddrinfo(addressResult);

    if(generalSocket == INVALID_SOCKET)
        throw ClientException(COULD_NOT_CREATE_CONNECTION);

    *this->out << "Socket has been successfully created." << std::endl;
#endif
}

const void Client::start() throw(ClientException) {
    this->globalInterrupt = false;

    auto bind = std::bind(&Client::readThreadInitialize, std::placeholders::_1);
    readThread = std::make_shared<std::thread>(bind, this);

    std::string clientMessage;

    while(!this->globalInterrupt) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        struct timeval time = {0, 10};
        select(STDIN_FILENO + 1, &set, NULL, NULL, &time);

        if(!FD_ISSET(STDIN_FILENO, &set))
            continue;

        std::getline(*this->in, clientMessage);

        if(clientMessage.empty())
            continue;

        if(clientMessage.back() != '\n')
            clientMessage.push_back('\n');

        if(clientMessage.size() > MESSAGE_SIZE)
            break;

        auto sendMessage = send(generalSocket, clientMessage.data(), (int) clientMessage.size(), EMPTY_FLAGS);

#ifdef _LINUX_
        if (sendMessage < 0)
#endif
#ifdef _WIN_
        if (sendMessage == SOCKET_ERROR)
#endif
            throw ClientException(COULD_NOT_SEND_MESSAGE);

        clientMessage.erase(std::remove(clientMessage.begin(), clientMessage.end(), ' '), clientMessage.end());
        clientMessage.erase(std::remove(clientMessage.begin(), clientMessage.end(), '\r'), clientMessage.end());
        clientMessage.erase(std::remove(clientMessage.begin(), clientMessage.end(), '\n'), clientMessage.end());
        std::transform(clientMessage.begin(), clientMessage.end(), clientMessage.begin(), ::tolower);

        if(clientMessage == "exit")
            break;
    }
}

void* Client::readThreadInitialize(void *thisPtr) {
    ((Client*)thisPtr)->feedbackExecutor();
    return NULL;
}

const void Client::feedbackExecutor() {
    while(true) {
        try {
            *this->out << readLine(generalSocket) << std::endl;
        }
        catch (const ClientException& exception) {
            this->globalInterrupt = true;
            break;
        }
    }
}

#ifdef _LINUX_
const std::string Client::readLine(const int socket) throw(ClientException) {
#endif
#ifdef _WIN_
const std::string Client::readLine(const SOCKET socket) throw(ClientException) {
#endif
    auto result = std::string();

    char resolvedSymbol = ' ';

#ifdef _LINUX_
    for(auto index = 0; index < MESSAGE_SIZE; ++index) {
        auto readSize = recv(socket, &resolvedSymbol, 1, EMPTY_FLAGS);
        if(readSize <= 0)
            throw ClientException(COULD_NOT_RECEIVE_MESSAGE);
        else if(resolvedSymbol == '\n')
            break;
        else if(resolvedSymbol != '\r')
            result.push_back(resolvedSymbol);
    }
#endif
#ifdef _WIN_
    while(true) {
        auto readSize = recv(socket, &resolvedSymbol, 1, EMPTY_FLAGS);
        if(readSize == 0)
            throw ClientException(COULD_NOT_RECEIVE_MESSAGE);
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

const void Client::stop() throw(ClientException) {
    if(readThread != nullptr && readThread->joinable())
        readThread->join();

#ifdef _LINUX_
    if(generalSocket <= 0)
        return;

    auto shutdownSocket = shutdown(generalSocket, SHUT_RDWR);
    if (shutdownSocket != 0)
        new ClientException(COULD_NOT_SHUT_SOCKET_DOWN);

    auto socketClose = close(generalSocket);
    if(socketClose != 0)
        throw ClientException(COULD_NOT_CLOSE_SOCKET);

    *this->out << "Socket is closed." << std::endl;

#endif
#ifdef _WIN_
    if(generalSocket <= 0 && generalWSAStartup != 0)
        return;

    auto shutdownSocket = shutdown(generalSocket, SD_SEND);
    if (shutdownSocket == SOCKET_ERROR)
        new ClientException(COULD_NOT_SHUT_SOCKET_DOWN);

    closesocket(generalSocket);
    generalSocket = INVALID_SOCKET;

    *this->out << "Socket is closed." << std::endl;

    if(generalWSAStartup != 0)
        return;

    WSACleanup();
    generalWSAStartup = -1;

    *this->out << "WSA has been successfully cleaned." << std::endl;
#endif
}

Client::~Client() {
    stop();
}

Client::ClientException::ClientException(const Client::Error error) {
    this->error = error;
}

const char* Client::ClientException::what() const noexcept {
    switch(this->error) {
        case COULD_NOT_STARTUP:
            return "It's impossible to startup WSA.";

        case COULD_NOT_RESOLVE_ADDRESS:
            return "It's impossible to resolve address.";

        case COULD_NOT_CREATE_SOCKET:
            return "It's impossible to create a socket.";

        case COULD_NOT_CREATE_CONNECTION:
            return "It's impossible to create connection (server is down?).";

        case COULD_NOT_SEND_MESSAGE:
            return "It's impossible to send message to server.";

        case COULD_NOT_RECEIVE_MESSAGE:
            return "It's impossible to receive server message.";

        case COULD_NOT_SHUT_SOCKET_DOWN:
            return "It's impossible to shut socket down.";

        case COULD_NOT_CLOSE_SOCKET:
            return "It's impossible to close socket.";
    }

    return "Unknown exception.";
}

const int Client::ClientException::code() const {
    return error;
}