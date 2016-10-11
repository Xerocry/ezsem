#include "../headers/ServerCommand.h"

ServerCommand::ServerCommand(std::string* expr, Server::ServerController* controller) : Command(expr, controller) { }

ServerCommand::~ServerCommand() { }

const void ServerCommand::parseAndExecute() const throw(CommandException, Server::ServerController::ControllerException) {
    auto commandPartition = prepareCommand();

    char* eventName;
    char* userName;
    std::chrono::milliseconds start;
    std::chrono::seconds period;

    switch(commandPartition.size()){
        case 1:
            if(commandPartition[0] == "help")
                this->controller->help(nullptr);
            else if(commandPartition[0] == "exit")
                this->controller->exit();
            else if(commandPartition[0] == "save")
                this->controller->save();
            else if(commandPartition[0] == "load")
                this->controller->load();
            else
                throw CommandException(COULD_NOT_RESOLVE_COMMAND);
            break;
        case 2:
            if(commandPartition[1] == "events" && commandPartition[0] == "info")
                this->controller->printEventsInfo(nullptr);
            else if(commandPartition[1] == "users" && commandPartition[0] == "info")
                this->controller->printUsersInfo();
            else if(commandPartition[1] == "accounts" && commandPartition[0] == "info")
                this->controller->printAccountsInfo();
            else if(commandPartition[1] == "subscriptions" && commandPartition[0] == "info")
                this->controller->printSubscriptionsInfo();
            else if(commandPartition[0] == "filename") {
                checkFilename(commandPartition[1]);
                this->controller->finit(commandPartition[1].data());
            }
            else if(commandPartition[0] == "detach") {
                userName = getNameFromCommand(commandPartition[1], false);
                this->controller->detach(userName);
            }
            else if(commandPartition[0] == "delete")
                this->controller->del(checkASCII(commandPartition[1]).data());
            else
                throw CommandException(COULD_NOT_RESOLVE_COMMAND);
            break;
        case 3:
            if(commandPartition[1] == "user" && commandPartition[0] == "info") {
                userName = getNameFromCommand(commandPartition[2], false);
                this->controller->printSelfInfo(nullptr, userName);
            }
            else if(commandPartition[0] == "register")
                this->controller->reg(checkASCII(commandPartition[1]).data(), checkASCII(commandPartition[2]).data());
            else if(commandPartition[0] == "event" && (commandPartition[1] == "drop" || commandPartition[1] == "notify")) {
                eventName = getNameFromCommand(commandPartition[2], true);
                if(commandPartition[1] == "drop") this->controller->eventDrop(eventName);
                else this->controller->eventNotify(eventName);
            }
            else
                throw CommandException(COULD_NOT_RESOLVE_COMMAND);
            break;
        case 4:
            if(commandPartition[0] == "event" && (commandPartition[1] == "subscribe" || commandPartition[1] == "unsubscribe")) {
                eventName = getNameFromCommand(commandPartition[2], true);
                userName = getNameFromCommand(commandPartition[3], false);
                if(commandPartition[1] == "subscribe") this->controller->eventSubscribe(eventName, userName);
                else this->controller->eventUnsubscribe(eventName, userName);
            }
            else
                throw CommandException(COULD_NOT_RESOLVE_COMMAND);
            break;
        case 5:
            if(commandPartition[0] == "event" && commandPartition[1] == "create" && commandPartition[2] == "single") {
                eventName = const_cast<char*>(checkASCII(commandPartition[3]).data());
                start = parseDate(commandPartition[4]);
                this->controller->eventCreate(eventName, start, std::chrono::seconds(0));
            }
            else
                throw CommandException(COULD_NOT_RESOLVE_COMMAND);
            break;
        case 6:
            if(commandPartition[0] == "event" && commandPartition[1] == "create" && commandPartition[2] == "multi") {
                eventName = const_cast<char*>(checkASCII(commandPartition[3]).data());
                start = parseDate(commandPartition[4]);
                try { period = std::chrono::seconds(std::stoi(commandPartition[5])); }
                catch(...) { throw CommandException(COULD_NOT_RESOLVE_ARGUMENT); }
                if(period < std::chrono::seconds(static_cast<int>(MIN_EVENT_PERIOD)) || period > std::chrono::seconds(static_cast<int>(MAX_EVENT_PERIOD)))
                    throw CommandException(ARGUMENT_LOGIC_ERROR);
                this->controller->eventCreate(eventName, start, period);
            }
            else
                throw CommandException(COULD_NOT_RESOLVE_COMMAND);
            break;
        default:
            throw CommandException(COULD_NOT_RESOLVE_COMMAND);
    }

}