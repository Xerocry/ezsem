#include "../headers/ServerCommand.h"

ServerCommand::ServerCommand(std::string* expr, Server::ServerController* controller) throw(CommandException) : Command(expr, controller) { }

ServerCommand::~ServerCommand() { }

const void ServerCommand::parseAndExecute() const throw(CommandException) {
    auto commandPartition = prepareCommand();

    char* eventName;

    switch(commandPartition.size()){
        case 1:
            if(commandPartition[0] == "exit")
                this->controller->exit();
            break;
        case 2:
            break;
        case 3:
            if(commandPartition[0] == "event" && commandPartition[1] == "drop") {
                try {
                    eventName = const_cast<char*>(this->controller->getEventNameByEventId(parseEventId(commandPartition[2])));
                } catch (const std::invalid_argument &exception) {
                    eventName = const_cast<char*>(commandPartition[3].data());
                } catch (const std::out_of_range &exception) {
                    throw CommandException(Error::COULD_NOT_RESOLVE_ARGUMENT, this->expr->data());
                }

                //this->controller->eventDrop(eventName);
            }
            break;
        case 4:
            break;
        case 5:
            break;
        default:
            new CommandException(Error::COULD_NOT_RESOLVE_COMMAND, this->expr->data());
    }

}