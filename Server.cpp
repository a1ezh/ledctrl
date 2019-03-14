#include "Server.h"

#include <iostream>
#include <thread>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>
#include <ctype.h>

using namespace std;


const std::string Server::FIFO_NAME = "ledctrl";


Server::Server(const string &fifoDir) : m_fifoDir(fifoDir) {
    m_fifoPath = m_fifoDir + "/" + FIFO_NAME;

    removeFifo();

    if (mkfifo(m_fifoPath.c_str(), S_IRUSR | S_IWUSR) == -1) {
        int err = errno;
        cerr << "Failed mkfifo " << m_fifoPath << ": " <<
                strerror(err) << endl;
        throw logic_error("mkfifo failed");
    }

    m_fifo.open(m_fifoPath);
    if (!m_fifo) {
        cerr << "Failed to open FIFO " << m_fifoPath << ": " << endl;
        removeFifo();
        throw logic_error("Failed to open FIFO");
    }

    m_dummy.open(m_fifoPath);
    if (!m_dummy) {
        cerr << "Failed to open FIFO " << m_fifoPath << ": " << endl;
        removeFifo();
        throw logic_error("Failed to open FIFO");
    }
}


void Server::processClient(pid_t pid) {
    cout << "> new client [" << pid << "]" << endl;

    const string inPath = m_fifoPath + ".in." + to_string(pid),
            outPath = m_fifoPath + ".out." + to_string(pid);

    ifstream in(inPath);
    ofstream out(outPath);

    if (!in) {
        cerr << "Failed to open FIFO " << inPath << ": " << endl;
        return;
    }

    for (string line, result; getline(in, line); ) {
        if (processCommand(line, result)) {
            out << "OK" << (result.empty() ? "" : " " + result) << endl;
            cout << m_ledControl << endl;
        } else
            out << "FAILED" << endl;
    }

    cout << "< client processed [" << pid << "]" << endl;
}


void Server::process() {
    pid_t pid;

    for (string line; getline(m_fifo, line); ) {
        if (!getPid(line, pid))
            continue;

        thread clientHandler(&Server::processClient, this, pid);
        clientHandler.detach();
    }
}


bool Server::getPid(const string &line, pid_t &pid) {
    try {
        pid = stoi(line);
    }
    catch (const logic_error &e) {
        cerr << "Bad PID recieved \"" << line << "\": " << e.what() << endl;
        return false;
    }
    return true;
}


bool Server::parseCommand(const string &line, string &command, string &arg) {
    auto wbeg = find_if(line.begin(), line.end(), ::isalpha);
    if (wbeg == line.end()) {
        cerr << "Ill-formed command: \"" << line << "\"" << endl;
        return false;
    }
    auto wend = find_if(wbeg, line.end(), ::isspace);
    command.assign(wbeg, wend);

    if (wend != line.end()) {
        wbeg = find_if(wend, line.end(), ::isalnum);
        if (wbeg != line.end()) {
            wend = find_if(wbeg, line.end(), ::isspace);
            arg.assign(wbeg, wend);
        }
    }

    return true;
}


bool Server::processCommand(const string &line, string &result) {
    result.clear();

    string command, arg;
    if (!parseCommand(line, command, arg))
        return false;

    auto checkArgExists = [&arg, &command, &line]() {
        if (arg.empty()) {
            cerr << "Ill-formed \"" << command << "\". No arg: \"" << line << "\"" << endl;
            return false;
        }
        return true;
    };

    if (command.compare("set-led-state") == 0) {
        if (!checkArgExists())
            return false;

        LedControl::State state;
        if (arg.compare("on") == 0)
            state = LedControl::State::On;
        else if (arg.compare("off") == 0)
            state = LedControl::State::Off;
        else {
            cerr << "Ill-formed \"" << command << "\": Invalid arg: \"" << line << "\"" << endl;
            return false;
        }

        m_ledControl.setState(state);
    } else if (command.compare("get-led-state") == 0) {
        result = (m_ledControl.getState() == LedControl::State::On ? "on" : "off");
    } else if (command.compare("set-led-color") == 0) {
        if (!checkArgExists())
            return false;

        LedControl::Color color;
        if (arg.compare("red") == 0)
            color = LedControl::Color::Red;
        else if (arg.compare("green") == 0)
            color = LedControl::Color::Green;
        else if (arg.compare("blue") == 0)
            color = LedControl::Color::Blue;
        else {
            cerr << "Ill-formed \"" << command << "\": Invalid arg: \"" << line << "\"" << endl;
            return false;
        }

        m_ledControl.setColor(color);
    } else if (command.compare("get-led-color") == 0) {
        auto color = m_ledControl.getColor();
        if (color == LedControl::Color::Red)
            result = "red";
        else if (color == LedControl::Color::Green)
            result = "green";
        else if (color == LedControl::Color::Blue)
            result = "blue";
        else {
            cerr << "Ill-formed \"" << command << "\": Invalid arg: \"" << line << "\"" << endl;
            return false;
        }
    } else if (command.compare("set-led-rate") == 0) {
        if (!checkArgExists())
            return false;

        bool convertFailed = false;
        unsigned long rate;
        try {
            rate = stoul(arg);
        } catch (const logic_error &) {
            convertFailed = true;
        }

        if (convertFailed || rate > LedControl::RATE_MAX) {
            cerr << "Ill-formed \"" << command << "\": Invalid arg: \"" << line << "\"" << endl;
            return false;
        }
        m_ledControl.setRate(rate);
    } else if (command.compare("get-led-rate") == 0) {
        result = to_string(m_ledControl.getRate());
    } else {
        cerr << "Unsupported command: \"" << line << "\"" << endl;
        return false;
    }

    return true;
}


void Server::removeFifo() {
    if (unlink(m_fifoPath.c_str()) == -1) {
        int err = errno;
        cerr << "Failed to remove FIFO " << m_fifoPath << ": " <<
                strerror(err) << endl;
    }
}


void Server::closeFifo(int fd) {
    if (close(fd) == -1) {
        int err = errno;
        cerr << "Failed to close FIFO " << m_fifoPath << ": " <<
                strerror(err) << endl;
    }
}
