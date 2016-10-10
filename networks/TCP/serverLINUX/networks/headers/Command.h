#ifndef NETWORKS_COMMAND_H
#define NETWORKS_COMMAND_H


#include <exception>
#include <string>
#include <vector>
#include "Server.h"

class Command {
protected:
    enum Error {
        COULD_NOT_RESOLVE_COMMAND = 0x1,
        COULD_NOT_RESOLVE_ARGUMENT = 0x2,
        ARGUMENT_LOGIC_ERROR = 0x3,
        COMMAND_OR_ARGUMENT_IS_TOO_LONG = 0x4
    };

    std::string* expr;
    Server::ServerController* controller;

public:
    static const int MAX_EVENT_PERIOD = 60 * 60 * 24 * 10;
    static const int MIN_EVENT_PERIOD = 10;
    static const int EVENT_BUFFER = 1;

    static const char PREFIX = '#';
    static const int MAX_LENGTH_OF_ARGUMENT = 30;

    class CommandException : public std::exception {
    private:
        Error error;
    public:
        explicit CommandException(const Error);
        const char* what() const noexcept override;
        const int code() const;
    };

protected:
    const std::vector<std::string> prepareCommand() const throw(CommandException);
    char* getNameFromCommand(const std::string& command, const bool isEvent) const throw(CommandException, Server::ServerController::ControllerException);
    explicit Command(std::string* expr, Server::ServerController* controller) throw(std::invalid_argument);
    virtual ~Command();

public:
    static const void checkFilename(const std::string &filename) throw(CommandException);
    static const std::string& checkASCII(const std::string &password) throw(CommandException);
    static const std::chrono::milliseconds parseDate(const std::string &string) throw(CommandException);
    static const int parseId(const std::string &stringId) throw(std::invalid_argument, std::out_of_range);
};


#endif //NETWORKS_COMMAND_H
