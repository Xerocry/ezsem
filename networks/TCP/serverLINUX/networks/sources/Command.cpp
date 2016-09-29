#include <algorithm>
#include "../headers/Command.h"

Command::Command(std::string* expr, Server::ServerController* controller) throw(CommandException) {
    this->expr = expr;
    this->controller = controller;
}

Command::~Command() {}

const std::vector<std::string> Command::prepareCommand() const {
    auto result = std::vector<std::string>();

    auto iterator = this->expr->begin();
    auto string = std::string();

    while(iterator != this->expr->end()) {
        if(*iterator == ' ' && !string.empty()){
            std::transform(string.begin(), string.end(), string.begin(), ::tolower);
            result.push_back(string);
            string.clear();
        }
        else if(*iterator != ' ')
            string.push_back(*iterator);

        ++iterator;
    }

    if(!string.empty())
        result.push_back(string);

    return result;
}

char* Command::getEventName(const std::string& command) const throw(CommandException) {
    char* result;

    try { result = const_cast<char*>(this->controller->getEventNameByEventId(parseEventId(command))); }
    catch (const std::invalid_argument &exception)
    { result = const_cast<char*>(command.data()); }
    catch (...)
    { throw CommandException(Error::COULD_NOT_RESOLVE_ARGUMENT); }

    return result;
}
char* Command::getLoginName(const std::string& command) const throw(CommandException) {
    char* result;

    try { result = const_cast<char*>(this->controller->getLoginByThreadId(parseThreadId(command))); }
    catch (const std::invalid_argument &exception)
    { result = const_cast<char*>(command.data()); }
    catch (...)
    { throw CommandException(Error::COULD_NOT_RESOLVE_ARGUMENT); }

    return result;
}

const int Command::parseEventId(const std::string& stringId) const throw(std::invalid_argument, std::out_of_range) {
    if(stringId[0] != EVENT_PREFIX)
        throw std::invalid_argument(stringId);

    if(stringId.size() < 2)
        throw std::out_of_range(stringId);

    int result = NULL;
    try { result = std::stoi(stringId.substr(1, stringId.length())); }
    catch(...) { throw std::out_of_range(stringId); }

    if(result < 0)
        throw std::out_of_range(stringId);

    return result;
}

const int Command::parseThreadId(const std::string& stringId) const throw(std::invalid_argument, std::out_of_range) {
    if(stringId[0] != THREAD_PREFIX)
        throw std::invalid_argument(stringId);

    if(stringId.size() < 2)
        throw std::out_of_range(stringId);

    int result = NULL;
    try { result = std::stoi(stringId.substr(1, stringId.length())); }
    catch(...) { throw std::out_of_range(stringId); }

    if(result < 0)
        throw std::out_of_range(stringId);

    return result;
}

const void Command::parseAndExecute() const throw(CommandException) { }

Command::CommandException::CommandException(const Error error) {
    this->error = error;
}

const char* Command::CommandException::what() const noexcept {
    switch(this->error){
        case COULD_NOT_RESOLVE_COMMAND:
            return "It's impossible to resolve command.";
        case COULD_NOT_RESOLVE_ARGUMENT:
            return "It's impossible to resolve arguments.";
        case COULD_NOT_PROVIDE_ACCESS:
            return "It's impossible to provide access to command.";
    }

    return "Unknown exception.";
}

const int Command::CommandException::code() const { return this->error; }