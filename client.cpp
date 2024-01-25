#include <iostream>
#include <string.h>
#include <thread>
#include "client.h"

namespace {
    int result = 0;
    TCPClient* client = nullptr;
    std::chrono::milliseconds period;

    class InvalidUseException: public std::exception {};

    void init (int argc, char* argv[]) {

        if (argc != 4) throw InvalidUseException();

        std::string name    = argv[1];
        int port            = std::stoi(argv[2]);
        int period_seconds  = std::stoi(argv[3]);
        period = std::chrono::milliseconds(period_seconds*1000);
        client = new TCPClient(name, port);
    }

    void loop () {

        while (true) {

            auto start = std::chrono::system_clock::now();
            client->run();
            auto end = start+period;
            std::this_thread::sleep_until(end);
        }
    }

    void clear () {

        delete client; 
        client=nullptr;
    }
}

int main (int argc, char* argv[]) {

    try {
        init(argc, argv);
        loop();
        clear();
    }
    catch (InvalidUseException&) {

        std::cerr << "Использование: " << argv[0] << "<Имя> <Порт> <Период(секунды)>" << '\n';
        clear();
    }
    catch (std::exception& e) {

        result = -1;
        std::cerr<<e.what()<<'\n';
        clear();        
    }
    catch (...) {
        result = -2;
        std::cerr<<"Unknown exception " << strerror(errno) << '\n';
        clear();
    }
    return result;
}