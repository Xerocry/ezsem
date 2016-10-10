#include <algorithm>
#include <sstream>
#include <iomanip>
#include "../headers/Command.h"

Command::Command(std::string* expr, Server::ServerController* controller) throw(std::invalid_argument) {
    if(expr == nullptr || controller == nullptr)
        throw std::invalid_argument("Pointer couldn't be null.");

    this->expr = expr;
    this->controller = controller;
}

Command::~Command() { }

const std::vector<std::string> Command::prepareCommand() const throw(CommandException){
    auto result = std::vector<std::string>();

    auto iterator = this->expr->begin();
    auto string = std::string();

    while(iterator != this->expr->end()) {
        if(*iterator == ' ' && !string.empty()){
            if(string.length() > MAX_LENGTH_OF_ARGUMENT)
                throw CommandException(COMMAND_OR_ARGUMENT_IS_TOO_LONG);
            std::transform(string.begin(), string.end(), string.begin(), ::tolower);
            result.push_back(string);
            string.clear();
        }
        else if(*iterator != ' ')
            string.push_back(*iterator);

        ++iterator;
    }

    if(!string.empty()) {
        if(string.length() > MAX_LENGTH_OF_ARGUMENT)
            throw CommandException(COMMAND_OR_ARGUMENT_IS_TOO_LONG);
        result.push_back(string);
    }

    return result;
}

char* Command::getNameFromCommand(const std::string& command, const bool isEvent) const throw(CommandException, Server::ServerController::ControllerException) {
    char* result;

    try {
        result = (isEvent) ? const_cast<char*>(this->controller->getEventNameByEventId(parseId(command))) :
                             const_cast<char*>(this->controller->getUserNameByThreadId(parseId(command)));
    }
    catch (const Server::ServerController::ControllerException& exception)
    { throw Server::ServerController::ControllerException(exception); }
    catch (const std::invalid_argument &exception)
    { result = const_cast<char*>(checkASCII(command).data()); }
    catch (...)
    { throw CommandException(COULD_NOT_RESOLVE_ARGUMENT); }

    return result;
}

const std::chrono::milliseconds Command::parseDate(const std::string &string) throw(CommandException) {
    std::tm time = {};
    std::stringstream stream(string);
    stream >> std::get_time(&time, "%d/%m/%Y|%H:%M:%S");

    if(stream.fail())
        throw CommandException(COULD_NOT_RESOLVE_ARGUMENT);

    auto result = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(std::mktime(&time)).time_since_epoch());
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    auto buffer = EVENT_BUFFER;

    if(result <= now + std::chrono::seconds(buffer))
        throw CommandException(ARGUMENT_LOGIC_ERROR);

    return result;
}

const std::string& Command::checkASCII(const std::string &value) throw(CommandException) {
    for(auto& current: value)
        if(current < '0' || current > 'z' || (current < 'a' && current > '9'))
            throw CommandException(COULD_NOT_RESOLVE_ARGUMENT);

    return value;
}

const void Command::checkFilename(const std::string &filename) throw(CommandException) {
    if(filename.find_first_of("\\/:*?\"<>|") != std::string::npos)
        throw CommandException(COULD_NOT_RESOLVE_ARGUMENT);
}

const int Command::parseId(const std::string &stringId) throw(std::invalid_argument, std::out_of_range) {
    if(stringId[0] != PREFIX)
        throw std::invalid_argument(stringId);

    if(stringId.size() < 2)
        throw std::out_of_range(stringId);

    auto string = stringId.substr(1, stringId.length());
    if(string.find_first_not_of("0123456789") != std::string::npos)
        throw std::out_of_range(stringId);

    int result;
    try { result = std::stoi(string); }
    catch(...) { throw std::out_of_range(stringId); }

    return result;
}

Command::CommandException::CommandException(const Error error) {
    this->error = error;
}

const char* Command::CommandException::what() const noexcept {
    switch(this->error){
        case COULD_NOT_RESOLVE_COMMAND:
            return "It's impossible to resolve command.";

        case COULD_NOT_RESOLVE_ARGUMENT:
            return "It's impossible to resolve argument.";

        case ARGUMENT_LOGIC_ERROR:
            return "Argument logic exception.";

        case COMMAND_OR_ARGUMENT_IS_TOO_LONG:
            return "It's impossible to resolve so long command or argument.";
    }

    return "Unknown exception.";
}

const int Command::CommandException::code() const { return this->error; }