#ifndef NETWORKS_SERVERCOMMAND_H
#define NETWORKS_SERVERCOMMAND_H


#include "Command.h"

class ServerCommand : public Command {
public:
    explicit ServerCommand(std::string* expr, Server::ServerController* controller);
    ~ServerCommand();

    const void parseAndExecute() const throw(CommandException, Server::ServerController::ControllerException);
};


#endif //NETWORKS_SERVERCOMMAND_H
