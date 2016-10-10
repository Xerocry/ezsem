#include "../headers/ClientCommand.h"

ClientCommand::ClientCommand(std::string* expr, Server::ServerController* controller, const int clientSocket) throw(Server::ServerController::ControllerException)  : Command(expr, controller){
    this->clientSocket = clientSocket;
}

ClientCommand::~ClientCommand() { }

const void ClientCommand::parseAndExecute(std::ostream* out) const throw(CommandException, Server::ServerController::ControllerException) {
    auto commandPartition = prepareCommand();

    char* eventName;
    char* userName;
    std::chrono::milliseconds start;
    std::chrono::seconds period;

    switch(commandPartition.size()) {
        case 1:
            if(commandPartition[0] == "help")
                this->controller->help(out);
            else if(commandPartition[0] == "exit")
                this->controller->close(this->clientSocket);
            else
                throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
            break;
        case 2:
            if(commandPartition[1] == "events" && commandPartition[0] == "info")
                this->controller->printEventsInfo(out);
            else if(commandPartition[1] == "self" && commandPartition[0] == "info")
                this->controller->printSelfInfo(out, this->controller->getUserNameBySocket(this->clientSocket));
            else
                throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
            break;
        case 3:
            if(commandPartition[0] == "register")
                this->controller->reg(checkASCII(commandPartition[1]).data(), checkASCII(commandPartition[2]).data());
            else if(commandPartition[0] == "connect")
                this->controller->connect(checkASCII(commandPartition[1]).data(), checkASCII(commandPartition[2]).data(), this->clientSocket);
            else if(commandPartition[0] == "event" && commandPartition[1] == "drop") {
                this->controller->getUserNameBySocket(this->clientSocket);
                eventName = getNameFromCommand(commandPartition[2], true);
                this->controller->eventDrop(eventName);
            }
            else if(commandPartition[0] == "event" && (commandPartition[1] == "subscribe" || commandPartition[1] == "unsubscribe")) {
                userName = const_cast<char*>(this->controller->getUserNameBySocket(this->clientSocket));
                eventName = getNameFromCommand(commandPartition[2], true);
                if(commandPartition[1] == "subscribe") this->controller->eventSubscribe(eventName, userName);
                else this->controller->eventUnsubscribe(eventName, userName);
            }
            else
                throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
            break;
        case 4:
                throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
        case 5:
            if(commandPartition[0] == "event" && commandPartition[1] == "create" && commandPartition[2] == "single") {
                this->controller->getUserNameBySocket(this->clientSocket);
                eventName = const_cast<char*>(checkASCII(commandPartition[3]).data());
                start = parseDate(commandPartition[4]);
                this->controller->eventCreate(eventName, true, start, std::chrono::seconds(0));
            }
            else
                throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
            break;
        case 6:
            if(commandPartition[0] == "event" && commandPartition[1] == "create" && commandPartition[2] == "multi") {
                this->controller->getUserNameBySocket(this->clientSocket);
                eventName = const_cast<char*>(checkASCII(commandPartition[3]).data());
                start = parseDate(commandPartition[4]);
                try { period = std::chrono::seconds(std::stoi(commandPartition[5])); }
                catch(...) { throw CommandException(Error::COULD_NOT_RESOLVE_ARGUMENT); }
                if(period < std::chrono::seconds(static_cast<int>(MIN_EVENT_PERIOD)) || period > std::chrono::seconds(static_cast<int>(MAX_EVENT_PERIOD)))
                    throw CommandException(Error::ARGUMENT_LOGIC_ERROR);
                this->controller->eventCreate(eventName, false, start, period);
            }
            else
                throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
            break;
        default:
            throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
    }

}