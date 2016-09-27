#ifndef NETWORKS_COMMAND_H
#define NETWORKS_COMMAND_H


#include <exception>
#include <string>
#include "Server.h"

class Command {
protected:
    enum Error {
        COULD_NOT_RESOLVE_COMMAND = 0x1,
        COULD_NOT_RESOLVE_ARGUMENT = 0x2,
        COULD_NOT_PROVIDE_ACCESS = 0x3
    };

    std::string* expr;
    Server::ServerController* controller;

public:
    class CommandException : public std::exception {
    private:
        Error error;
        char* command;
    public:
        explicit CommandException(const Error, const char*);
        const char* what() const noexcept override;
        const int code() const;
    };

protected:
    explicit Command(std::string* expr, Server::ServerController* controller) throw(CommandException);
    virtual ~Command();

public:
    virtual const void parseAndExecute() const throw(CommandException);
};


#endif //NETWORKS_COMMAND_H
