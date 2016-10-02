#ifndef NETWORKS_SERVERCOMMAND_H
#define NETWORKS_SERVERCOMMAND_H


#include "Command.h"
#include "Server.h"

class ServerCommand : public Command {
public:
    explicit ServerCommand(std::string* expr, Server::ServerController* controller);
    ~ServerCommand();
public:
    const void parseAndExecute() const throw(CommandException, Server::ServerController::ControllerException) override;
};


#endif //NETWORKS_SERVERCOMMAND_H
