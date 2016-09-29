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
        COULD_NOT_PROVIDE_ACCESS = 0x3
    };

    static const int MAX_COMMAND_PARTITION_SIZE = 5;
    static const char EVENT_PREFIX = 'e';
    static const char THREAD_PREFIX = 't';

    std::string* expr;
    Server::ServerController* controller;

public:
    class CommandException : public std::exception {
    private:
        Error error;
        char* command;
    public:
        explicit CommandException(const Error);
        const char* what() const noexcept override;
        const int code() const;
    };

protected:
    char* getEventName(const std::string& command) const throw(CommandException);
    char* getLoginName(const std::string& command) const throw(CommandException);
    const std::vector<std::string> prepareCommand() const;
    const int parseEventId(const std::string& stringId) const throw(std::invalid_argument, std::out_of_range);
    const int parseThreadId(const std::string& stringId) const throw(std::invalid_argument, std::out_of_range);
    explicit Command(std::string* expr, Server::ServerController* controller) throw(CommandException);
    virtual ~Command();

public:
    virtual const void parseAndExecute() const throw(CommandException);
};


#endif //NETWORKS_COMMAND_H
