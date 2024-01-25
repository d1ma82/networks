#include <iostream>
#include "server.h"

namespace {

    class InvalidUseException: public std::exception {};

    int result = 0;
    TCPServer* server = nullptr;

    void init (int argc, char* argv[]) {

        if (argc != 2) throw InvalidUseException();
        
        int port = std::stoi(argv[1]);
        std::cout<<"Порт: "<<port<<'\n';
        server = new TCPServer(port, "log.txt");
    }

    void loop () {

        while (true) {

            server->start_accept();
        }
    }

    void clear () {

        if (server) { delete server; server = nullptr; }
    }
}

int main (int argc, char* argv[]) {

    try {
        init(argc, argv);
        loop();
        clear();
    }
    catch (InvalidUseException& e) {

        std::cerr << "Использование: " << argv[0] << "<Порт>" << '\n';
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
