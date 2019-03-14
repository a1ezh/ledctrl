#ifndef SERVER_H
#define SERVER_H

#include "LedControl.h"
#include <string>
#include <fstream>

class Server {
public:
    Server(const std::string &fifoDir = "/tmp");

    ~Server() {
        removeFifo();
    }

    // Main worker. Listens for client PIDs and create thread for each one.
    void process();

private:
    // Thread worker for each incoming client
    void processClient(pid_t pid);

    bool getPid(const std::string &line, pid_t &pid);

    // Parse command and argument
    bool parseCommand(const std::string &line, std::string &command, std::string &arg);

    bool processCommand(const std::string &line, std::string &result);

    void removeFifo();

    void closeFifo(int fd);

    static const std::string FIFO_NAME;
    std::string m_fifoDir;
    std::string m_fifoPath;
    std::ifstream m_fifo;
    std::ofstream m_dummy;
    LedControl m_ledControl;
};




#endif
