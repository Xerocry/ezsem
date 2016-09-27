#include "../headers/Command.h"

Command::Command(std::string* expr, Server::ServerController* controller) throw(CommandException) {
    this->expr = expr;
    this->controller = controller;
}

Command::~Command() {}

const void Command::parseAndExecute() const throw(CommandException) { }

Command::CommandException::CommandException(const Error, const char *) { }

const char* Command::CommandException::what() const noexcept { return nullptr; }

const int Command::CommandException::code() const { return NULL; }