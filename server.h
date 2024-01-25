#pragma once

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <memory>
#include <vector>
#include <thread>
#include <sstream>
#include <fstream>
#include <mutex>
#include <arpa/inet.h>

namespace {

std::mutex file_mutex;

void write_file (const char* logfile, const char* data) {

    std::lock_guard<std::mutex> lock(file_mutex);
    std::ofstream file(logfile, std::ios::app);
    
    //TODO: Parse data
    if (file.is_open()) {

        std::istringstream iss(data);
        std::string word;

        while (iss >> word) {

            if (word.empty()) continue;

            if (word.front()=='"' && word.back()=='"') 
                        word = word.substr(1, word.length()-2);
            
            file << word << ' ';
        }
        file << '\n'; 
        file.close(); 
    } else {
        std::cerr << "Unable to open file: " << logfile << std::endl;
    }
}

struct ConnectionArg {

    socklen_t client_sock;
    sockaddr_in client_address; 
    const char* logfile;
};

class Connection {
private:
    ConnectionArg args;

public:
    Connection(const ConnectionArg& args): args{args} {}

    ~Connection() { std::cout<<"Destroy Connection\n"; }

    void do_stuff() {
        
        ssize_t bytes_write =   0;
        ssize_t bytes_read  =   0;

        std::stringstream client;
        client << inet_ntoa(args.client_address.sin_addr)
               <<':'<<ntohs(args.client_address.sin_port)
               <<':'<<args.client_sock;

        try {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));

            std::string message("Привет от сервера\n");

            strncpy(buffer, message.c_str(), sizeof(buffer) - 1);
            bytes_write = send(args.client_sock, buffer, strlen(buffer), 0);

            int read=0;
            memset(buffer, 0, sizeof(buffer));
            while ((read = recv(args.client_sock, buffer, sizeof(buffer), 0)) > 0) {
                
                bytes_read += read;
                buffer[read] = '\0'; 

                write_file(args.logfile, buffer);
                std::cout << "Получено от "<< client.str() << " сообщение: " << buffer << std::endl;
                memset(buffer, 0, sizeof(buffer));
            }
        }
        catch (...) {

            std::cerr << "Произошла ошибка в потоке клиента " << client.str() << std::endl;
        }
         
        std::cout << "Клиент "<< client.str() << " всего: отправлено " << bytes_write << " получено " << bytes_read << '\n';
        close(args.client_sock);
    }
};

/**
 *  Потоко безопасное создание новых подключений
*/
class ConnectionManager {
private:
    std::mutex mutex;
    std::unique_ptr<Connection> connection;
    
public:
    ConnectionArg args;

    ConnectionManager(): connection(nullptr) {}

    std::unique_ptr<Connection> acquire() {
        
        std::lock_guard<std::mutex> lock(mutex);
        if (!connection) connection = std::make_unique<Connection>(args);
        return std::move(connection);
    }
};

class TCPServer {
private:
    int server_sock {0};
    sockaddr_in server_address;
    ConnectionManager manager;

    void connection_run(ConnectionManager& manager) {

        auto connection = manager.acquire();
        connection->do_stuff();
    }

public:
    TCPServer (short port, const char* logfile) {

        manager.args.logfile = logfile;
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(port);

        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock == -1) {
            std::cerr << "Ошибка при создании сокета" << std::endl;
            throw std::exception();
        }

            // Привязка сокета к адресу
        if (bind(server_sock, (sockaddr*)& server_address, sizeof(server_address)) == -1) {

            std::cerr << "Ошибка при привязке сокета к адресу" << std::endl;
            close(server_sock);
            throw std::exception();
        }

                    // Прослушивание порта
        if (listen(server_sock, 5) == -1) {

            std::cerr << "Ошибка при прослушивании порта" << std::endl;
            close(server_sock);
            throw std::exception();
        }
        std::cout << "Ожидание подключений..." << std::endl;
    }

    ~TCPServer() { 

        if (server_sock>0) close(server_sock); 
    }

    void start_accept() {
     
        socklen_t client_sock_length = sizeof(manager.args.client_address);          
        
        manager.args.client_sock = accept(
            server_sock, 
            (sockaddr*)& manager.args.client_address, 
            &client_sock_length
        );

        if (manager.args.client_sock == -1) {
        
                std::cerr << "Ошибка при принятии подключения" << std::endl;
                throw std::exception();
        }

        std::cout<<"Подключение из " << 
                inet_ntoa(manager.args.client_address.sin_addr) << ':' <<
                ntohs(manager.args.client_address.sin_port)<<
                ':' <<manager.args.client_sock << '\n';
        
        std::thread thread = std::thread(&TCPServer::connection_run, this, std::ref(manager));
        thread.detach();    
    }
};
}