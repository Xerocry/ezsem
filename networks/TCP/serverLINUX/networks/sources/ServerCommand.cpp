#include "../headers/ServerCommand.h"

ServerCommand::ServerCommand(std::string* expr, Server::ServerController* controller) throw(CommandException) : Command(expr, controller) { }

ServerCommand::~ServerCommand() { }

const void ServerCommand::parseAndExecute() const throw(CommandException) {
    if(*expr == "exit")
        controller->exit();
}