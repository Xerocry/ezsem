#ifndef NETWORKS_CLIENTCOMMAND_H
#define NETWORKS_CLIENTCOMMAND_H

#include "Command.h"

class ClientCommand : public Command {
private:
#ifdef _LINUX_
    int clientSocket;
#endif
#ifdef _WIN_
    SOCKET clientSocket;
#endif

public:
#ifdef _LINUX_
    explicit ClientCommand(std::string* expr, Server::ServerController* controller, const int clientSocket) throw(Server::ServerController::ControllerException);
#endif
#ifdef _WIN_
    explicit ClientCommand(std::string* expr, Server::ServerController* controller, const SOCKET clientSocket) throw(Server::ServerController::ControllerException);
#endif
    ~ClientCommand();

    const void parseAndExecute(std::ostream* out) const throw(CommandException, Server::ServerController::ControllerException);
};


#endif //NETWORKS_CLIENTCOMMAND_H
