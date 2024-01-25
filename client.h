#pragma once
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <arpa/inet.h>

namespace {

class TCPClient {
private:
    int client_socket;
    sockaddr_in server_address;
    std::string name;
    int period;
    short port;

    void prepare_and_send() {
        
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        tm time_info;
        localtime_r(&time, &time_info);
        char buffer[20];

        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_info);

        // get milliseconds
        auto milliseconds = 
            std::chrono::duration_cast
                    <std::chrono::milliseconds>
                            (now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << "[" << buffer << "." <<  milliseconds.count() << "] " << name;
        
        int len = strlen(oss.str().c_str());
        if (send(client_socket, oss.str().c_str(), len, 0) == -1) {

            std::cerr << "Ошибка при отправке данных" << std::endl;
            close(client_socket);
            return;
        }

        std::cout << "Данные успешно отправлены на сервер." << std::endl;
        close(client_socket);
    }

public:
    TCPClient(const std::string& name, short port): name{name}, port{port} {

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_address.sin_port = htons(port);
    }

    void run() {

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
                
                std::cerr << "Ошибка при создании сокета" << std::endl;
                throw std::exception();
        }

        if (connect(
            client_socket, 
            (sockaddr*)& server_address, 
            sizeof(server_address))==-1) {

                std::cerr << "Ошибка при подключении к серверу" << std::endl;
                close(client_socket);
                throw std::exception();
        }

        prepare_and_send();
    }
};

}