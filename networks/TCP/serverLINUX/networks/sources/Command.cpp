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

const int Command::parseEventId(const std::string& stringId) const throw(std::invalid_argument, std::out_of_range) {
    if(stringId.size() < 2)
        throw std::invalid_argument(stringId);

    if(stringId[0] != EVENT_PREFIX)
        throw std::invalid_argument(stringId);

    auto result = std::stoi(stringId.substr(1, stringId.length()));
    if(result < 0)
        throw std::invalid_argument(stringId);

    return result;
}

const int Command::parseThreadId(const std::string& stringId) const throw(std::invalid_argument, std::out_of_range) {
    if(stringId.size() < 2)
        throw std::invalid_argument(stringId);

    if(stringId[0] != THREAD_PREFIX)
        throw std::invalid_argument(stringId);

    auto result = std::stoi(stringId.substr(1, stringId.length()));
    if(result < 0)
        throw std::invalid_argument(stringId);

    return result;
}

const void Command::parseAndExecute() const throw(CommandException) { }

Command::CommandException::CommandException(const Error, const char *) { }

const char* Command::CommandException::what() const noexcept { return nullptr; }

const int Command::CommandException::code() const { return NULL; }