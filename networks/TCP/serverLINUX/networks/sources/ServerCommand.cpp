#include "../headers/ServerCommand.h"

ServerCommand::ServerCommand(std::string* expr, Server::ServerController* controller) throw(CommandException) : Command(expr, controller) { }

ServerCommand::~ServerCommand() { }

const void ServerCommand::parseAndExecute() const throw(CommandException) {
    auto commandPartition = prepareCommand();

    char* eventName, userName;

    switch(commandPartition.size()){
        case 1:
            if(commandPartition[0] == "exit")
                this->controller->exit();
            break;
        case 2:
            break;
        case 3:
            if(commandPartition[0] == "event" && (commandPartition[1] == "drop" || commandPartition[1] == "notify")) {
                eventName = getEventName(commandPartition[2]);

                if(commandPartition[1] == "drop") this->controller->eventDrop(eventName);
                else this->controller->eventNotify(eventName);
            }
            break;
        case 4:
            if(commandPartition[0] == "event" && (commandPartition[1] == "subscribe" || commandPartition[1] == "unsubscribe"))
            {

            }
            break;
        case 5:
            break;
        default:
            throw CommandException(Error::COULD_NOT_RESOLVE_COMMAND);
    }

}