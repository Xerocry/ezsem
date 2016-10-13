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

#ifdef _TCP_
    generalSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
#ifdef _UDP_
    generalSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
    if(generalSocket < 0)
        throw ClientException(COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

#ifdef _TCP_
    struct sockaddr_in serverAddress;
#endif
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(address);

#ifdef _UDP_
    sendto(generalSocket, CONNECT_STRING, strlen(CONNECT_STRING), EMPTY_FLAGS, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

    if(readLine() != std::string(ACCEPT_STRING))
        throw ClientException(COULD_NOT_CREATE_CONNECTION);

    *this->out << "Connection established." << std::endl;
#endif

#ifdef _TCP_
    auto serverConnection = connect(generalSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(serverConnection < 0)
        throw ClientException(COULD_NOT_CREATE_CONNECTION);

    *this->out << "Connection established." << std::endl;
#endif

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

#ifdef _WIN_
#endif

    while(!this->globalInterrupt) {
#ifdef _LINUX_
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        struct timeval time = {0, 10};
        select(STDIN_FILENO + 1, &set, NULL, NULL, &time);

        if(!FD_ISSET(STDIN_FILENO, &set))
            continue;
#endif

        std::getline(*this->in, clientMessage);

#ifdef _WIN_
        if(this->globalInterrupt)
            break;
#endif

        if(clientMessage.empty())
            continue;

        if(clientMessage.back() != '\n')
            clientMessage.push_back('\n');

        if(clientMessage.size() > MESSAGE_SIZE)
            break;

#ifdef _TCP_
        auto sendMessage = send(generalSocket, clientMessage.data(), (int) clientMessage.size(), EMPTY_FLAGS);
#endif
#ifdef _UDP_
        auto sendMessage = sendto(generalSocket, clientMessage.data(), (int) clientMessage.size(), EMPTY_FLAGS, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
#endif

#ifdef _LINUX_
        if (sendMessage < 0)
#endif
#ifdef _WIN_
        if (sendMessage == SOCKET_ERROR)
#endif
            throw ClientException(COULD_NOT_SEND_MESSAGE);
    }
}

void* Client::readThreadInitialize(void *thisPtr) {
    ((Client*)thisPtr)->feedbackExecutor();
    return NULL;
}

const void Client::feedbackExecutor() {
    while(true) {
        try {
            *this->out << readLine() << std::endl;
        }
        catch (const ClientException& exception) {
            this->globalInterrupt = true;
#ifdef _WIN_
            *this->out << "Connection lost. Press \"Enter\" to exit." << std::endl;
#endif
            break;
        }
    }
}

const std::string Client::readLine() throw(ClientException) {
    auto result = std::string();

#ifdef _LINUX_

#ifdef _TCP_
    char resolvedSymbol = ' ';

    for(auto index = 0; index < MESSAGE_SIZE; ++index) {
        auto readSize = recv(generalSocket, &resolvedSymbol, 1, EMPTY_FLAGS);
        if(readSize <= 0)
            throw ClientException(COULD_NOT_RECEIVE_MESSAGE);
        else if(resolvedSymbol == '\n')
            break;
        else if(resolvedSymbol != '\r')
            result.push_back(resolvedSymbol);
    }
#endif
#ifdef _UDP_
    char input[MESSAGE_SIZE];
    bzero(input, sizeof(input));
    auto size = sizeof(serverAddress);
    recvfrom(generalSocket, input, MESSAGE_SIZE, EMPTY_FLAGS, (struct sockaddr*) &serverAddress, (socklen_t*) &size);
    result = input;

    if(result.back() == '\n')
        result.erase(result.size() - 1);

    if(result == std::string(DETACH_STRING)) {
        sendto(generalSocket, DETACH_STRING, strlen(DETACH_STRING), EMPTY_FLAGS, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
        throw ClientException(COULD_NOT_RECEIVE_MESSAGE);
    }
#endif

#endif

#ifdef _WIN_
    char resolvedSymbol = ' ';

    while(true) {
        auto readSize = recv(generalSocket, &resolvedSymbol, 1, EMPTY_FLAGS);
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
    if(!this->globalInterrupt)
#ifdef _TCP_
        send(generalSocket, "exit", 4, EMPTY_FLAGS);
#endif
#ifdef _UDP_
        sendto(generalSocket, "exit", 4, EMPTY_FLAGS, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
#endif

    if(readThread != nullptr && readThread->joinable())
        readThread->join();

#ifdef _LINUX_
    if(generalSocket <= 0)
        return;

#ifdef _TCP_
    auto shutdownSocket = shutdown(generalSocket, SHUT_RDWR);
    if (shutdownSocket != 0)
        new ClientException(COULD_NOT_SHUT_SOCKET_DOWN);
#endif

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