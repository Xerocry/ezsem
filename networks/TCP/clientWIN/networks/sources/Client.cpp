#include "../headers/Client.h"

// For getaddrinfo() definition.

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

// End getaddrinfo() include.

#ifndef _STRUCT_ANSI_
    #define _STRUCT_ANSI_ 0
#endif
#include <stdlib.h>

#include <cstring>

Client::Client(std::ostream* out, std::istream* in, const char* port, const char* address) throw(ClientException) {
    this->in = in;
    this->out = out;

    WSADATA wsaData;
    generalWSAStartup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(generalWSAStartup != 0)
        throw ClientException(Error::COULD_NOT_STARTUP);

    *this->out << "WSA has been successfully started." << std::endl;

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *addressResult = nullptr;
    auto wsaAddress = getaddrinfo(address, port, &hints, &addressResult);
    if (wsaAddress != 0)
        throw ClientException(Error::COULD_NOT_RESOLVE_ADDRESS);

    *this->out << "Address and port have been successfully resolved." << std::endl;

    for(auto currentPtr = addressResult; currentPtr != nullptr; currentPtr = currentPtr->ai_next){
        generalSocket = socket(currentPtr->ai_family, currentPtr->ai_socktype, currentPtr->ai_protocol);
        if(generalSocket == INVALID_SOCKET)
            throw ClientException(Error::COULD_NOT_CREATE_SOCKET);

        auto serverConnection = connect(generalSocket, currentPtr->ai_addr, (int) currentPtr->ai_addrlen);
        if(serverConnection != SOCKET_ERROR)
            break;

        closesocket(generalSocket);
        generalSocket = INVALID_SOCKET;
    }

    freeaddrinfo(addressResult);

    if(generalSocket == INVALID_SOCKET)
        throw ClientException(Error::COULD_NOT_CREATE_CONNECTION);

    *this->out << "Socket has been successfully created." << std::endl;

    char resultState[strlen(CLIENT_JOINED) + 1];
    do {
        memset(resultState, 0, sizeof resultState);
        auto readSize = recv(generalSocket, resultState, strlen(CLIENT_JOINED), EMPTY_FLAGS);
        resultState[strlen(CLIENT_JOINED)] = '\0';
        if(readSize < 0)
            throw ClientException(Error::COULD_NOT_RECEIVE_MESSAGE);
    } while(strcmp((char*) resultState, CLIENT_JOINED) != 0);

    *this->out << "Connection established." << std::endl;
}

const void Client::start() const throw(ClientException){
    *this->out << "Ready to send messages." << std::endl;

    char clientMessage[MESSAGE_SIZE], serverMessage[MESSAGE_SIZE];
    while(true) {
        memset(clientMessage, 0, sizeof clientMessage);
        *this->out << "Send message to server: ";
        *this->in >> clientMessage;

        auto messageLength = strlen(clientMessage);
        if(messageLength == 0) {
            *this->out << "Empty message. Nothing to send." << std::endl;
            continue;
        }

        auto sendMessage = send(generalSocket, clientMessage, messageLength, EMPTY_FLAGS);
        if (sendMessage == SOCKET_ERROR)
            new ClientException(Error::COULD_NOT_SEND_MESSAGE);

        *this->out << "Waiting for a response..." << std::endl;

        while(true) {
            memset(serverMessage, 0, sizeof serverMessage);
            auto readSize = recv(generalSocket, serverMessage, MESSAGE_SIZE, EMPTY_FLAGS);

            if(readSize < 0)
                throw ClientException(Error::COULD_NOT_RECEIVE_MESSAGE);
            else if(readSize == 0)
                continue;

            *this->out << "Message \"" << serverMessage << "\" has been delivered to the server." << std::endl;
            break;
        }
    }
}

const void Client::stop() throw(ClientException){
    if(generalSocket <= 0 && generalWSAStartup != 0)
        return;

    auto shutdownSocket = shutdown(generalSocket, SD_SEND);
    if (shutdownSocket == SOCKET_ERROR)
        new ClientException(Error::COULD_NOT_SHUT_SOCKET_DOWN);

    closesocket(generalSocket);
    generalSocket = INVALID_SOCKET;

    *this->out << "Socket is closed." << std::endl;

    if(generalWSAStartup != 0)
        return;

    WSACleanup();
    generalWSAStartup = -1;

    *this->out << "WSA has been successfully cleaned." << std::endl;
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
    }

    return "Unknown exception.";
}

const int Client::ClientException::code() const {
    return error;
}