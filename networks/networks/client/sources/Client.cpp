#include <algorithm>
#include <string.h>
#include <sstream>
#include "../headers/Client.h"


#if defined(_LINUX_) || defined(_UDP_)
Client::Client(std::ostream* out, std::istream* in, const uint16_t port, const char* address) throw(ClientException) {
#endif
#if defined(_WIN_) && defined(_TCP_)
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
    this->generalInterrupt = false;

    try { writeLine(ATTACH_STRING, true); }
    catch (const ClientException& exception) {
        throw ClientException(COULD_NOT_CREATE_CONNECTION);
    }

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

#ifdef _TCP_
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
#ifdef _UDP_
    generalSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (generalSocket == SOCKET_ERROR)
        throw ClientException(COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

    ZeroMemory(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.S_un.S_addr = inet_addr(address);

    try { writeLine(ATTACH_STRING, true); }
    catch (const ClientException& exception) {
        throw ClientException(COULD_NOT_CREATE_CONNECTION);
    }

    *this->out << "Connection established." << std::endl;
#endif

#endif
}

const void Client::start() throw(ClientException) {
    this->generalInterrupt = false;

    auto bind = std::bind(&Client::readThreadInitialize, std::placeholders::_1);
    readThread = std::make_shared<std::thread>(bind, this);

    std::string clientMessage;

#ifdef _WIN_
#endif

    while(!this->generalInterrupt) {
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
        if(this->generalInterrupt)
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
#ifdef _LINUX_
        if (sendMessage < 0)
#endif
#ifdef _WIN_
        if (sendMessage == SOCKET_ERROR)
#endif
            throw ClientException(COULD_NOT_SEND_MESSAGE);
#endif
#ifdef _UDP_
        writeLine(clientMessage, false);
#endif
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
            this->generalInterrupt = true;
#ifdef _WIN_
            *this->out << "Connection lost. Press \"Enter\" to exit." << std::endl;
#endif
            break;
        }
    }
}

#ifdef _UDP_
const void Client::writeLine(const std::string message, const bool special) throw(ClientException) {
    auto result = std::string(message);

    if(special) {
        currentPackageNumber = (message == std::string(ATTACH_STRING)) ? 0 : -1;
        if(currentPackageNumber == -1)
            return;
    }
    else {
        currentPackageNumber = progressivePackageNumber;
        ++progressivePackageNumber;
    }

    std::remove(result.begin(), result.end(), '\r');
    if(result.back() != '\n')
        result.push_back('\n');

    result.insert(0, "@");
    result.insert(0, std::to_string(currentPackageNumber));
    result.insert(0, SEND_STRING);

    responseArrived = false;

    for(auto tryIndex = 0; tryIndex < TRIES_COUNT; ++tryIndex) {
        sendto(generalSocket, result.data(), result.size(), EMPTY_FLAGS, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

        if(currentPackageNumber == 0 && readLine() == ATTACH_STRING)
            return;
        else if(currentPackageNumber != 0) {
            auto iterationsWait = ITERATIONS_COUNT;
            while (!responseArrived && iterationsWait != 0)
                --iterationsWait;

            if (responseArrived)
                return;
        }
    }

    throw ClientException(COULD_NOT_SEND_MESSAGE);
}
#endif

const std::string Client::readLine() throw(ClientException) {
    auto result = std::string();

#ifdef _UDP_

    int iterationIndex = 0;

    char input[MESSAGE_SIZE];
    while(!this->generalInterrupt) {
        if(currentPackageNumber == 0 && iterationIndex == 1) {
            result.clear();
            break;
        }

        ++iterationIndex;

        bzero(input, sizeof(input));
        auto size = sizeof(serverAddress);
        recvfrom(generalSocket, input, MESSAGE_SIZE, EMPTY_FLAGS, (struct sockaddr *) &serverAddress, (socklen_t *) &size);
        result = input;

        std::remove(result.begin(), result.end(), '\r');
        if (result.back() == '\n')
            result.pop_back();

        if (result.size() < 3)
            continue;

        auto prefix = result.substr(0, 2);

        if(prefix == std::string(SEND_STRING)) {
            auto response = result.substr(2, result.size() - 2);
            auto find = response.find_first_of('@', 0);

            if(find == std::string::npos || find >= response.size() - 1)
                continue;

            auto stream = std::stringstream(response.substr(0, find));

            int packageNumber;
            stream >> packageNumber;
            if(stream.fail())
                continue;

            if(packageNumber == 0)
                continue;

            auto message = std::string(result);

            result = response.substr(find + 1, response.size() - find - 1);

            response = std::string(RESPONSE_STRING) + std::to_string(packageNumber);
            sendto(generalSocket, response.data(), response.size(), EMPTY_FLAGS, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

            if(packageNumber == 1) {
                if(result == std::string(DETACH_STRING)) {
                    sendto(generalSocket, message.data(), message.size(), EMPTY_FLAGS, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
                    throw ClientException(COULD_NOT_RECEIVE_MESSAGE);
                }
                else
                    continue;
            }

            break;
        }
        else if(prefix == std::string(RESPONSE_STRING)) {
            auto stream = std::stringstream(result.substr(2, result.size() - 2));

            int packageNumber;
            stream >> packageNumber;
            if(stream.fail())
                continue;

            if(packageNumber != currentPackageNumber)
                continue;

            responseArrived = true;

            if(packageNumber == 0 && currentPackageNumber == 0) {
                result = ATTACH_STRING;
                break;
            }

            continue;
        }
        else
            continue;
    }

#endif

#ifdef _TCP_
#ifdef _LINUX_

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
#endif

    return result;
}

const void Client::stop() throw(ClientException) {
    if(!this->generalInterrupt)
#ifdef _TCP_
        send(generalSocket, "exit\n", 5, EMPTY_FLAGS);
#endif
#ifdef _UDP_
        writeLine("exit", false);
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

#ifdef _TCP_
    auto shutdownSocket = shutdown(generalSocket, SD_SEND);
    if (shutdownSocket == SOCKET_ERROR)
        new ClientException(COULD_NOT_SHUT_SOCKET_DOWN);
#endif

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