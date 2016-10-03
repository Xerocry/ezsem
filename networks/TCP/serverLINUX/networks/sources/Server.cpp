#include "../headers/Server.h"
#include "../headers/ServerCommand.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iomanip>

Server::Server(std::ostream* out, std::istream* in, std::ostream* error, const uint16_t port, const char* filename) throw(ServerException) {
    this->out = out;
    this->in = in;
    this->error = error;
    this->filename = std::string(filename);

    generalSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(generalSocket < 0)
        throw ServerException(Error::COULD_NOT_CREATE_SOCKET);

    *this->out << "Socket has been successfully created." << std::endl;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    generalBind = bind(generalSocket, (sockaddr *) &serverAddress, sizeof(serverAddress));
    if(generalBind < 0)
        throw ServerException(Error::COULD_NOT_BIND);

    *this->out << "Socket has been successfully bind." << std::endl;

    generalFlags = fcntl(generalSocket, F_GETFL, 0);
    auto fcntlResult = fcntl (generalSocket, F_SETFL, generalFlags | O_NONBLOCK);
    if (fcntlResult < 0)
        throw ServerException(Error::COULD_NOT_SET_NON_BLOCKING);

    listen(generalSocket, BACKLOG);

    *this->out << "Listen socket." << std::endl;
}

const void Server::start() throw(ServerException, ServerController::ControllerException) {
    this->events = std::map<int, Server::Event*>();
    this->users = std::map<int, Server::User*>();
    this->accounts = std::map<std::string, std::string>();
    this->timings = std::vector<std::pair<int, std::chrono::milliseconds>>();
    this->subscriptions = std::vector<std::pair<std::string, std::string>>();

    this->generalInterrupt = false;

    this->controller = new ServerController(this);
    this->controller->load();

    auto bind = std::bind(&Server::commandThreadInitialize, std::placeholders::_1);
    commandThread = std::make_shared<std::thread>(bind, this);

    while(!this->generalInterrupt) {
        sockaddr_in clientAddress;
        auto size = sizeof(struct sockaddr_in);
        auto clientSocket = accept(generalSocket, (sockaddr *) &clientAddress, (socklen_t *) &size);
        if (clientSocket >= 0)
            createClientThread(clientSocket, clientAddress.sin_addr);
    }
}

void* Server::timerThreadInitialize(void *thisPtr) {
    ((Server*)thisPtr)->eventTimer();
    return NULL;
}

const void Server::eventTimer() {
    while(!this->generalInterrupt && !this->timerInterrupt) {
        if(this->timings.empty())
            continue;

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

        auto resultVector = std::vector<int>();

        for(auto& current: this->timings) {
            if (now >= current.second)
                resultVector.push_back(current.first);
            else
                break;
        }

        for(auto& current: resultVector) {
            refreshTiming(current);
            auto currentTime = time(0);
            struct tm* nowStruct = localtime(&currentTime);
            *this->out << nowStruct->tm_hour << ":" << nowStruct->tm_min << ":" << nowStruct->tm_sec << " Handle event #" << current << "." << std::endl;
        }
    }
}

void* Server::commandThreadInitialize(void *thisPtr) {
    ((Server*)thisPtr)->commandExecutor();
    return NULL;
}

const void Server::commandExecutor() {
    std::string command;
    while(!this->generalInterrupt){
        std::getline(*this->in, command);

        try {
            ServerCommand(&command, controller).parseAndExecute();
            *this->out << "Command has been successfully executed." << std::endl;
        }
        catch(const Command::CommandException& exception) {
            *this->error << exception.what() << std::endl;
        }
        catch(const Server::ServerController::ControllerException& exception) {
            *this->error << exception.what() << std::endl;
        }
        catch(const std::exception& exception) {
            *this->error << exception.what() << std::endl;
        }
        catch(...) {
            *this->error << "Strange resolve command error." << std::endl;
        }
    }
}

const void Server::refreshTiming(const int eventId) {
    std::chrono::milliseconds timing;
    bool eraseCompleted = false;

    for(auto current = this->timings.begin(); current != this->timings.end(); ++current)
        if(current->first == eventId) {
            timing = current->second;
            this->timings.erase(current);
            eraseCompleted = true;
            break;
        }

    if(this->events.find(eventId) == this->events.end())
        return;

    auto event = this->events.at(eventId);

    if(eraseCompleted) {
        if(event->period.count() == 0) {
            this->events.erase(eventId);
            return;
        }
        timing += event->period;
    }
    else
        timing = event->startMoment;

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    auto buffer = Command::EVENT_BUFFER;

    if(now + std::chrono::seconds(buffer) >= timing) {
        if(event->period.count() == 0) {
            this->events.erase(eventId);
            return;
        }
        else {
            auto count = (now + std::chrono::seconds(buffer) - timing) / event->period;
            timing += (count + 1) * event->period;
        }
    }

    bool insertCompleted = false;

    for(auto current = this->timings.begin(); current != this->timings.end(); ++current)
        if(current->second >= timing) {
            this->timings.insert(current, std::pair<int, std::chrono::milliseconds>(eventId, timing));
            insertCompleted = true;
            break;
        }

    if(!insertCompleted)
        this->timings.insert(this->timings.end(), std::pair<int, std::chrono::milliseconds>(eventId, timing));
}

void* Server::clientThreadInitialize(void *thisPtr, const int threadId, const int clientSocket, const struct in_addr address) {
    auto serverPtr = ((Server*)thisPtr);

    try { serverPtr->acceptClient(threadId, clientSocket, address); }
    catch (const ServerException& exception) {
        *serverPtr->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
    }
    try {
        serverPtr->removeClientThread(threadId);
        *serverPtr->out << "Thread 0x" << threadId << ". Client disconnected." << std::endl;
    } catch (const ServerException& exception) {
        *serverPtr->error << "Thread 0x" << threadId << ". " << exception.what() << std::endl;
    }

    return NULL;
}

const void Server::acceptClient(const int threadId, const int clientSocket, const struct in_addr address) throw(ServerException){
    *this->out << "Thread 0x" << threadId << ". Client with IP address " << inet_ntoa(address) << " has been connected." << std::endl;

    write(clientSocket, CommunicationConstants::CLIENT_JOINED, strlen(CommunicationConstants::CLIENT_JOINED));

    char message[MESSAGE_SIZE];
    bzero(message, sizeof message);

    while(true) {
        auto readSize = recv(clientSocket, message, MESSAGE_SIZE, EMPTY_FLAGS);

        if(readSize < 0)
            throw ServerException(Error::COULD_NOT_RECEIVE_MESSAGE);
        else if(readSize == 0)
            break;

        *this->out << "Thread 0x" << threadId << ". Received message: " << message << std::endl;

        write(clientSocket, message, strlen(message));
        bzero(message, sizeof message);
    }
}

const void Server::createClientThread(const int clientSocket, const struct in_addr address) {
    int index = 0;
    std::map<int, Server::User*>::iterator result;
    do {
        result = this->users.find(index);
        ++index;
    } while(result != this->users.end());
    --index;

    auto bind = std::bind(&Server::clientThreadInitialize, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    auto user = new User;
    user->userName = "name";
    user->thread = std::make_shared<std::thread>(bind, this, index, clientSocket, address);
    user->socket = clientSocket;
    this->users.insert(std::pair<int, Server::User*>(index, user));
}

const void Server::removeClientThread(const int threadId) throw(ServerException) {
    const auto find = this->users.find(threadId);
    if(find != this->users.end()) {
        const auto clientSocket = find->second->socket;
        clearSocket(threadId, clientSocket);
        this->users.at(threadId)->thread.get()->detach();
        this->users.erase(threadId);
    }
}

const void Server::clearSocket(const int threadId, const int socket) throw(ServerException) {
    if(socket < 0)
        return;

    auto socketShutdown = shutdown(socket, SHUT_RDWR);
    if(socketShutdown != 0)
        throw ServerException(Error::COULD_NOT_SHUT_SOCKET_DOWN);

    *this->out <<  "Thread 0x" << threadId << ". Socket is down." << std::endl;

    auto socketClose = close(socket);
    if(socketClose != 0)
        throw ServerException(Error::COULD_NOT_CLOSE_SOCKET);

    *this->out <<  "Thread 0x" << threadId << ". Socket is closed." << std::endl;
}

const void Server::stop() throw(ServerException){
    this->generalInterrupt = true;
    this->timerInterrupt = true;

    if(generalSocket >= 0) {
        auto fcntlResult = fcntl(generalSocket, F_SETFL, generalFlags);
        if (fcntlResult < 0)
            throw ServerException(Error::COULD_NOT_SET_NON_BLOCKING);
    }

    if(timerThread != nullptr && timerThread.get()->joinable())
        timerThread.get()->join();

    if(commandThread != nullptr && commandThread.get()->joinable())
        commandThread.get()->join();

    for (auto &current: this->users) {
        auto temp = current.second->socket;
        current.second->socket = -1;
        clearSocket(current.first, temp);
        if (current.second->thread != nullptr && current.second->thread.get()->joinable())
            current.second->thread.get()->join();
    }

    clearSocket(-1, generalSocket);

    this->controller->finit(BACKUP_FILENAME);
    this->controller->save();
}

Server::~Server() {
    stop();
}

// ServerController

Server::ServerController::ServerController(Server *serverPtr) {
    this->serverPtr = serverPtr;
}

const int Server::ServerController::getThreadIdByUserName(const char* userName) const throw(ControllerException) {
    auto string = std::string(userName);
    for(auto& current: this->serverPtr->users)
        if(current.second->userName == string)
            return current.first;

    throw ControllerException(USER_IS_NOT_EXISTS);
}
const int Server::ServerController::getEventIdByEventName(const char *eventName) const throw(ControllerException) {
    auto string = std::string(eventName);
    for(auto& current: this->serverPtr->events)
        if(current.second->eventName == string)
            return current.first;

    throw ControllerException(EVENT_IS_NOT_EXISTS);
}
const char* Server::ServerController::getEventNameByEventId(const int eventId) const throw(ControllerException) {
    auto userFind = this->serverPtr->events.find(eventId);
    if(userFind == this->serverPtr->events.end())
        throw ControllerException(EVENT_IS_NOT_EXISTS);
    else
        return this->serverPtr->events.at(eventId)->eventName.data();
}
const char* Server::ServerController::getUserNameByThreadId(const int threadId) const throw(ControllerException) {
    auto userFind = this->serverPtr->users.find(threadId);
    if(userFind == this->serverPtr->users.end())
        throw ControllerException(USER_IS_NOT_EXISTS);
    else
        return this->serverPtr->users.at(threadId)->userName.data();
}

const void Server::ServerController::eventCreate(const char *eventName, const bool singleMode, const std::chrono::milliseconds& start, const std::chrono::seconds& period) const throw(ControllerException) {
    try { getEventIdByEventName(eventName); }
    catch (const ControllerException& exception) {
        int index = 0;
        std::map<int, Server::Event*>::iterator result;
        do {
            result = this->serverPtr->events.find(index);
            ++index;
        } while(result != this->serverPtr->events.end());
        --index;

        //event create multi some 03/10/2016|05:25:00 10

        auto event = new Server::Event;
        event->eventName = std::string(eventName);
        event->startMoment = start;
        event->period = period;
        this->serverPtr->events.insert(std::pair<int, Server::Event*>(index, event));

        this->serverPtr->refreshTiming(index);
        return;
    }

    throw ControllerException(EVENT_IS_ALREADY_EXISTS);
}
const void Server::ServerController::eventDrop(const char *eventName) const throw(ControllerException) {
    auto eventId = getEventIdByEventName(eventName);
    this->serverPtr->events.erase(eventId);
    this->serverPtr->refreshTiming(eventId);
}
const void Server::ServerController::eventNotify(const char *eventName) const {

}
const void Server::ServerController::eventSubscribe(const char *eventName, const char *userName) const throw(ControllerException) {

}
const void Server::ServerController::eventUnsubscribe(const char *eventName, const char *userName) const throw(ControllerException) {

}

const void Server::ServerController::printEventsInfo() const {
    if(this->serverPtr->events.empty())
        *this->serverPtr->out << "Events table is empty." << std::endl;
    else {
        *this->serverPtr->out << std::endl
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "ID"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "EVENTNAME"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "START"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "PERIOD"
                              << std::endl << std::endl;

        for (auto &current: this->serverPtr->events) {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> castPoint(current.second->startMoment);
            auto time = std::chrono::system_clock::to_time_t(castPoint);
            std::string timeString = std::ctime(&time);
            timeString.resize(timeString.size() - 1);

            __int64_t days, hours, minutes, seconds;
            __int64_t period = current.second->period.count();
            std::string result;
            if(period != 0) {
                days = period / (60 * 60 * 24);
                hours = (period % (60 * 60 * 24)) / (60 * 60);
                minutes = (period % (60 * 60)) / 60;
                seconds = period % 60;
                result = std::to_string(days) + " days " + std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
            }

            *this->serverPtr->out << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << (std::string(1, Command::PREFIX) + std::to_string(current.first))
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.second->eventName
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << timeString
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << ((period == 0) ? "Single event." : result)
                                  << std::endl;
        }

        *this->serverPtr->out << std::endl;
    }
}
const void Server::ServerController::printUsersInfo() const {
    if(this->serverPtr->users.empty())
        *this->serverPtr->out << "Users table is empty." << std::endl;
    else {
        *this->serverPtr->out << std::endl
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "ID"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "USERNAME"
                              << std::endl << std::endl;

        for (auto &current: this->serverPtr->users) {
            *this->serverPtr->out << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << (std::string(1, Command::PREFIX) + std::to_string(current.first))
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.second->userName
                                  << std::endl;
        }

        *this->serverPtr->out << std::endl;
    }
}
const void Server::ServerController::printAccountsInfo() const {
    if(this->serverPtr->accounts.empty())
        *this->serverPtr->out << "Accounts table is empty." << std::endl;
    else {
        *this->serverPtr->out << std::endl
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "USERNAME"
                              << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << "PASSWORD"
                              << std::endl << std::endl;

        for (auto &current: this->serverPtr->accounts) {
            *this->serverPtr->out << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.first
                                  << std::left << std::setw(Command::MAX_LENGTH_OF_ARGUMENT) << std::setfill(' ') << current.second
                                  << std::endl;
        }
        *this->serverPtr->out << std::endl;
    }
}

const void Server::ServerController::exit() const {
    this->serverPtr->generalInterrupt = true;
}
const void Server::ServerController::finit(const char *filename) const {
    this->serverPtr->filename = std::string(filename);
}
const void Server::ServerController::save() const throw(ControllerException) {
    auto filename = (this->serverPtr->filename.empty()) ? DEFAULT_FILENAME: this->serverPtr->filename;
    auto stream = std::ofstream(filename);

    if(!stream.is_open())
        throw ControllerException(COULD_NOT_OPEN_FILE);

    stream << this->serverPtr->events.size() << std::endl;
    for(auto& current: this->serverPtr->events)
        stream << current.second->eventName << std::endl << current.second->startMoment.count() << std::endl << current.second->period.count() << std::endl;

    stream << this->serverPtr->accounts.size() << std::endl;
    for(auto& currentAccount: this->serverPtr->accounts) {
        stream << currentAccount.first << std::endl << currentAccount.second << std::endl;

        int count = 0;
        for(auto& currentSubscribe: this->serverPtr->subscriptions)
            if(currentSubscribe.first == currentAccount.first)
                ++count;

        stream << count << std::endl;

        for(auto& currentSubscribe: this->serverPtr->subscriptions)
            if(currentSubscribe.first == currentAccount.first)
                stream << currentSubscribe.second << std::endl;
    }

    stream.close();
}
const void Server::ServerController::load() const throw(ControllerException) {
    if(this->serverPtr->timerThread != nullptr) {
        this->serverPtr->timerInterrupt = true;
        if(this->serverPtr->timerThread.get()->joinable())
            this->serverPtr->timerThread.get()->join();
    }

    this->serverPtr->timings.clear();
    this->serverPtr->events.clear();

    for(auto& current: this->serverPtr->users) {
        auto temp = current.second->socket;
        current.second->socket = -1;
        this->serverPtr->clearSocket(current.first, temp);
        if(current.second->thread != nullptr && current.second->thread.get()->joinable())
            current.second->thread.get()->join();
    }

    this->serverPtr->users.clear();

    auto filename = (this->serverPtr->filename.empty()) ? DEFAULT_FILENAME: this->serverPtr->filename;
    auto stream = std::ifstream(filename);

    if(!stream.is_open())
        throw ControllerException(COULD_NOT_OPEN_FILE);

    std::stringstream stringStream;

    std::string eventName;
    std::string startString, periodString;
    __int64_t start, period;

    std::string countString;
    int count = -1, index = 0;

    while(!stream.eof()) {
        if(count == -1) {
            std::getline(stream, countString);
            stringStream.clear();
            stringStream.str(countString);
            stringStream >> count;
        }
        else if(count > 0) {
            std::getline(stream, eventName);
            if(stream.eof()) throw ControllerException(COULD_NOT_PARSE_FILE);
            std::getline(stream, startString);
            stringStream.clear();
            stringStream.str(startString);
            stringStream >> start;
            if(stream.eof()) throw ControllerException(COULD_NOT_PARSE_FILE);
            std::getline(stream, periodString);
            stringStream.clear();
            stringStream.str(periodString);
            stringStream >> period;

            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            auto buffer = Command::EVENT_BUFFER;
            if(((period != 0) || (now + std::chrono::seconds(buffer) < std::chrono::milliseconds(start))) && Command::MAX_EVENT_PERIOD >= period && (period == 0 || Command::MIN_EVENT_PERIOD <= period)) {
                try {
                    auto event = new Server::Event;
                    event->eventName = Command::checkASCII(eventName);
                    event->startMoment = std::chrono::milliseconds(start);
                    event->period = std::chrono::seconds(period);
                    this->serverPtr->events.insert(std::pair<int, Server::Event*>(index, event));
                    ++index;
                } catch (const Command::CommandException &exception) {
                    throw ControllerException(COULD_NOT_PARSE_FILE);
                }
            }

            --count;
        }
        else
            break;
    }

    count = -1;

    std::string login, password;
    int subCount;

    while(!stream.eof()) {
        if(count == -1) {
            std::getline(stream, countString);
            stringStream.clear();
            stringStream.str(countString);
            stringStream >> count;
        }
        else if(count > 0) {
            std::getline(stream, login);
            if(stream.eof()) throw ControllerException(COULD_NOT_PARSE_FILE);
            std::getline(stream, password);
            if(stream.eof()) throw ControllerException(COULD_NOT_PARSE_FILE);
            std::getline(stream, countString);
            stringStream.clear();
            stringStream.str(countString);
            stringStream >> subCount;

            std::vector<std::string> resultSubs;
            std::string currentSub;
            for(auto subIndex = 0; subIndex < subCount; ++subIndex) {
                if(stream.eof()) throw ControllerException(COULD_NOT_PARSE_FILE);
                std::getline(stream, currentSub);

                bool containsInCurrent = false;
                for(auto& current: resultSubs)
                    if(currentSub == current) {
                        containsInCurrent = true;
                        break;
                    }

                bool containsInEvents = false;
                for(auto& current: this->serverPtr->events)
                    if(currentSub == current.second->eventName) {
                        containsInEvents = true;
                        break;
                    }

                if(!containsInCurrent && containsInEvents)
                    resultSubs.push_back(currentSub);
            }

            try {
                this->serverPtr->accounts.insert(std::pair<std::string, std::string>(Command::checkASCII(login), Command::checkASCII(password)));
                for(auto& current: resultSubs)
                    this->serverPtr->subscriptions.push_back(std::pair<std::string, std::string>(Command::checkASCII(login), Command::checkASCII(current)));
            } catch (const Command::CommandException& exception){
                throw ControllerException(COULD_NOT_PARSE_FILE);
            }
            --count;
        }
        else
            break;
    }

    stream.close();

    for(auto& current: this->serverPtr->events)
        this->serverPtr->refreshTiming(current.first);

    this->serverPtr->timerInterrupt = false;
    auto bind = std::bind(&Server::timerThreadInitialize, std::placeholders::_1);
    this->serverPtr->timerThread = std::make_shared<std::thread>(bind, this->serverPtr);
}
const void Server::ServerController::detach(const char *userName) const throw(ControllerException) {
    for(auto& current: this->serverPtr->users)
        if(current.second->userName == userName) {
            auto temp = current.second->socket;
            current.second->socket = -1;
            this->serverPtr->clearSocket(current.first, temp);
            if(current.second->thread != nullptr && current.second->thread.get()->joinable())
                current.second->thread.get()->join();
            return;
        }

    throw ControllerException(USER_IS_NOT_EXISTS);
}
const void Server::ServerController::del(const char *userName) const throw(ControllerException) {
    auto userFind = this->serverPtr->accounts.find(std::string(userName));
    if(userFind == this->serverPtr->accounts.end())
        throw ControllerException(USER_IS_NOT_EXISTS);
    else {
        try { detach(userName); }
        catch (const ControllerException& exception) {}

        for(auto current = this->serverPtr->subscriptions.begin(); current != this->serverPtr->subscriptions.end(); ++current)
            if(current->first == std::string(userName)) {
                this->serverPtr->subscriptions.erase(current);
                --current;
            }

        this->serverPtr->accounts.erase(userFind);
    }
}
const void Server::ServerController::reg(const char *userName, const char *password) const throw(ControllerException) {
    auto userFind = this->serverPtr->accounts.find(userName);
    if(userFind == this->serverPtr->accounts.end())
        this->serverPtr->accounts.insert(std::pair<std::string, std::string>(userName, password));
    else
        throw ControllerException(USER_IS_ALREADY_EXISTS);
}

// ServerController::ControllerException

Server::ServerController::ControllerException::ControllerException(const Server::ServerController::Error error) {
    this->error = error;
}

const char* Server::ServerController::ControllerException::what() const noexcept {
    switch(this->error) {
        case COULD_NOT_OPEN_FILE:
            return "It's impossible to open file.";

        case COULD_NOT_PARSE_FILE:
            return "It's impossible to parse file.";

        case USER_IS_ALREADY_EXISTS:
            return "User is already exists.";

        case USER_IS_NOT_EXISTS:
            return "User isn't exists.";

        case EVENT_IS_ALREADY_EXISTS:
            return "Event is already exists.";

        case EVENT_IS_NOT_EXISTS:
            return "Event isn't exists.";
    }

    return "Unknown exception.";
}

const int Server::ServerController::ControllerException::code() const {
    return this->error;
}

// ServerException

Server::ServerException::ServerException(const Server::Error error) {
    this->error = error;
}

const char* Server::ServerException::what() const noexcept {
    switch(this->error){
        case COULD_NOT_CREATE_SOCKET:
            return "It's impossible to create a socket.";

        case COULD_NOT_BIND:
            return "It's impossible to bind (Maybe server is already started on this port).";

        case COULD_NOT_SET_NON_BLOCKING:
            return "It's impossible to set non-blocking flag.";

        case COULD_NOT_ACCEPT:
            return "It's impossible to accept incoming message.";

        case COULD_NOT_RECEIVE_MESSAGE:
            return "It's impossible to receive message.";

        case COULD_NOT_SHUT_SOCKET_DOWN:
            return "It's impossible to shut socket down.";

        case COULD_NOT_CLOSE_SOCKET:
            return "It's impossible to close socket.";
    }

    return "Unknown exception.";
}

const int Server::ServerException::code() const {
    return this->error;
}
