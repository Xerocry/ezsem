#ifndef NETWORKS_CLIENTCOMMAND_H
#define NETWORKS_CLIENTCOMMAND_H


#include "Command.h"

class ClientCommand : public Command {
private:
    int clientSocket;

public:
    explicit ClientCommand(std::string* expr, Server::ServerController* controller, const int clientSocket) throw(Server::ServerController::ControllerException);
    ~ClientCommand();

    const void parseAndExecute(std::ostream* out) const throw(CommandException, Server::ServerController::ControllerException);
};


#endif //NETWORKS_CLIENTCOMMAND_H
