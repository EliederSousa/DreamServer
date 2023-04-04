#include <WinSock2.h>
#include <Windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include "include/logger.h"
#include "include/utils.h"
#include "include/ticker.h"
#include "include/iniloader.h"
#include "include/threadpool.h"

#define FMT_HEADER_ONLY
#include "include/fmt/core.h"
#include "include/fmt/color.h"
#include "include/fmt/chrono.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>

// g++ *.cpp -I.\include -s -Os -o server -lws2_32 -fconcepts

void clientThreadFunction( int clientID );

ThreadPool pool;
std::mutex iomutex;

namespace GameServer {
    class Client {
        public:
        SOCKADDR_IN client_info;
        SOCKET sock;
        std::string ip;
        int id;
        int port;
        int ticksWithNoInfo;
        bool isConnected;

        public:
        Client(){};
    };

    class Server {
        public:
            std::vector<int> disconnectedClients;

        private:
        /* Estas linhas garantem que as cores das letras aparecerão em qualquer console do Windows. */
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        bool boolConsoleModeA = GetConsoleMode(hOut, &dwMode);
        bool boolConsoleModeB = SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        
        int _maxClients = 2;
        int _port = 8000;
        std::string _ip = "127.0.0.1";
        int _refusedConnections = 0;
        int _bannedIPS = 0;
        int _serverWarnings = 0;
        bool _status = true;
        bool _debug = false;
        int _maxbuffersize = 1024*1024;
        std::time_t _startedTime = std::time(nullptr);

        std::vector<Client> _clients;

        WSADATA Winsockdata;
        SOCKET TCPServerSocket;
        struct sockaddr_in TCPServerAdd;
        struct sockaddr_in TCPClientAdd;

        int iTCPClientAdd = sizeof(TCPClientAdd);

        public:

        /*~Server() {
            for (int w=0; w<_clients.size(); w++ ) 
                closesocket(_clients[w].sock);
            closesocket(TCPServerSocket);
            WSACleanup();
        }*/

        Server() {
            int debug;
            IniLoader::loadINIFile( "config.ini" );
            IniLoader::getValue("serverconfig", "ip", _ip);
            IniLoader::getValue("serverconfig", "port", _port);
            IniLoader::getValue("serverconfig", "debugmode", debug);
            IniLoader::getValue("networkconfig", "maxclients", _maxClients);
            IniLoader::getValue("networkconfig", "maxbuffersize", _maxbuffersize);

            _debug = (bool)debug;

            Logger::startLoggingToFile();

            if (WSAStartup(MAKEWORD(2, 2),&Winsockdata) != 0 ) {
                Logger::serverError( "Couldn't start winsock lib." );
                consoleUpdate();
            } else {
                TCPServerAdd.sin_family = AF_INET;
                TCPServerAdd.sin_addr.s_addr = inet_addr( _ip.c_str() );
                TCPServerAdd.sin_port = htons( _port );
                TCPServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if( TCPServerSocket == INVALID_SOCKET ) {
                    Logger::serverError( fmt::format( "Error creating socket. Code: {}", WSAGetLastError() ) );
                    consoleUpdate();
                } else {
                    if( bind(TCPServerSocket, (SOCKADDR*)&TCPServerAdd, sizeof(TCPServerAdd)) == SOCKET_ERROR ) {
                        Logger::serverError( fmt::format( "Error binding socket. Code: {}", WSAGetLastError() ) );
                        consoleUpdate();
                    } else {
                        if( listen(TCPServerSocket, 2) == SOCKET_ERROR ) {
                            Logger::serverError( fmt::format( "Error listening socket. Code: {}", WSAGetLastError() ) );
                            consoleUpdate();
                        } else {
                            Logger::serverSuccess( fmt::format( "Server listening at {}:{}", _ip, _port) );
                            if ( _debug ) 
                                Logger::serverWarning("Server started in debug mode.");
                            consoleUpdate();
                        }
                    }
                }
            }
        }
        
        /**
         * @brief Send a message to specified client. 
         * 
         * @param cli The client who will receive the message.
         * @param msg A string containing a message.
         * @return bool Return true if the message could be sended or false otherwise.
         */
        bool sendMsg( Client cli, std::string msg ) {
            char SenderBuffer[512];
            std::copy(msg.begin(), msg.end(), SenderBuffer);
            std::size_t length = msg.copy(SenderBuffer, 512);
            if( length == 512 ) {
                SenderBuffer[511]='\0';
            } else {
                SenderBuffer[length]='\0';
            }

            int iSend = send(cli.sock, SenderBuffer, strlen( SenderBuffer ) + 1, 0);
            if( send(cli.sock, SenderBuffer, strlen( SenderBuffer ) + 1, 0) == SOCKET_ERROR ) {
                return false;
            } else {
                return true;
            }
        }


        bool getStatus() {
            return _status;
        }

        /**
         * @brief Checks if server is full (maximum capacity).
         * 
         * @return bool Returns true if server is full, or false otherwise.
         */
        bool isServerFull() {
            return _clients.size() >= _maxClients;
        }

        /**
         * @brief Checks if server was started in Debug Mode. This allows clients with the same IP connect more than one time in the server.
         * 
         * @return bool Returns true if it is in Debug Mode, or false otherwise.
         */
        bool isDebugMode() {
            return _debug;
        }

        void enableDebugMode() {
            _debug = true;
        }

        void disableDebugMode() {
            _debug = false;
        }

        bool acceptCon( GameServer::Client &cli ) {
            cli.client_info = {0};
            int addrsize = sizeof(cli.client_info);
            cli.sock = accept(TCPServerSocket, (struct sockaddr*)&cli.client_info, &addrsize);
            char *ip = inet_ntoa(cli.client_info.sin_addr);

            cli.ip = inet_ntoa(cli.client_info.sin_addr);
            cli.port = (int)(cli.client_info.sin_port);

            if( cli.sock == INVALID_SOCKET ) {
                return false;
            }
            return true;
        }

        std::string get_all_buf(int sock) {
            int n = 1, total = 0, found = 0;
            char c;
            char temp[1024*256]; 

            // Keep reading up to a '\n'

            while (!found) {
                n = recv(sock, &temp[total], sizeof(temp) - total - 1, 0);
                std::cout << temp << std::endl;
                if (n < 0) {
                    /* Error, check 'errno' for more details */
                    break;
                }
                total += n;
                temp[total] = '\0';
                found = (strchr(temp, '\n') != 0);
            }

            std::string s = temp;
            return s.substr(0, s.size()-1); // Remove the breakline of our protocol
        }

        std::string recvMsg(int sock) {
            return get_all_buf(sock);
        }

        /**
         * @brief Get the number of connected clients in the server.
         * 
         * @return int The number of connected clients in server.
         */
        int getNumClients() {
            return _clients.size();
        }

        /**
         * @brief Checks if a particular IP address is connected to the server.
         * 
         * @param IP A string with the IP address
         * @return bool Returns true if the IP is connected, false otherwise.
         */
        bool isIPConnected( std::string IP ) {
            for( int w=0; w<_clients.size(); w++ ) {
                if ( _clients[w].ip == IP ) 
                    return true;
            }
            return false;
        }

        /**
         * @brief Add a client if the server is not full.
         * 
         * @param clientParam The client to be added.
         * @return bool Return true if the client was added successfully, false otherwise.
         */
        bool addClient( Client clientParam ) {
            if( !isServerFull() ) {
                int clientid = IDGenerator::generate();
                clientParam.id = clientid;
                clientParam.ticksWithNoInfo = 0;
                clientParam.isConnected = true;
                
                _clients.push_back( clientParam );           
                pool.addThread( &clientThreadFunction, clientid );  
                return true;
            } else {
                return false;
            }
        }

        GameServer::Client getClient( int id ) {
            for ( auto& cli : _clients ) {
                if (cli.id == id) {
                    return cli;
                }
            }
            Logger::serverError(fmt::format("There is no client with ID {}", id));
        }

        bool removeClient( int idParam ) {
            for( int w=0; w<_clients.size(); w++ ){
                if( _clients[w].id == idParam ) {
                    _clients.erase( _clients.begin() + w );
                }
            }
            return false;
        }

        bool isBannedIP( Client clientParam ) {
            return false;
        }

        void addRefusedConnection() {
            _refusedConnections++;
        }

        void consoleUpdate() {
            DWORD written = 0;
            PCWSTR sequence = L"\x1b[2J";
            WriteConsoleW(hOut, sequence, (DWORD)wcslen(sequence), &written, NULL);

            std::cout << std::endl;
            fmt::print(fg(fmt::color::dim_gray), "      ____  ____  _________    __  ________ __________ _    ____________  \n");
            fmt::print(fg(fmt::color::dark_slate_blue), "     / __ \\/ __ \\/ ____/   |  /  |/  / ___// ____/ __ \\ |  / / ____/ __ \\ \n");
            fmt::print(fg(fmt::color::royal_blue),      "    / / / / /_/ / __/ / /| | / /|_/ /\\__ \\/ __/ / /_/ / | / / __/ / /_/ / \n");
            fmt::print(fg(fmt::color::light_sea_green), "   / /_/ / _, _/ /___/ ___ |/ /  / /___/ / /___/ _, _/| |/ / /___/ _, _/  \n");
            fmt::print(fg(fmt::color::aquamarine),      "  /_____/_/ |_/_____/_/  |_/_/  /_//____/_____/_/ |_| |___/_____/_/ |_|   \n\n");
            fmt::print(fmt::emphasis::bold | fg(fmt::color::yellow_green), "                          Created by DreamTeam, 2023\n\n");

            Logger::printQueue();

            PROCESS_MEMORY_COUNTERS_EX pmc;
            GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
            SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

            float memoryComsuption = (float)virtualMemUsedByMe / 1048576.0;
            std::time_t now = std::time(nullptr);

            std::string network = "OK";

            std::string ipport = _ip;
            ipport += ":";
            ipport += std::to_string(_port);

            fmt::print(fg(fmt::color::silver), "\n •───────────────────────────────────────────────────────────────────────•\n");
            fmt::print(fg(fmt::color::silver), " │  ");
            fmt::print(fg(fmt::color::lime),     "✓"); // ✓
            fmt::print(fg(fmt::color::silver), fmt::format("{: >2}/{: <2} ", getNumClients(), _maxClients)); 
            fmt::print(fg(fmt::color::crimson), "⛒"); // ⛒
            fmt::print(fg(fmt::color::silver),   fmt::format("{: >2} ", _refusedConnections ) );
            fmt::print(fg(fmt::color::gold),    "⚠"); // ⚠
            fmt::print(fg(fmt::color::silver),   fmt::format("{: >2} ", _serverWarnings ) );
            fmt::print(fg(fmt::color::orange),  "⚡"); // ⚡
            fmt::print(fg(fmt::color::silver),   fmt::format(" {:3.2}MB  ", memoryComsuption ) );
            fmt::print(fg(fmt::color::violet),  "⇅");
            fmt::print(fg(fmt::color::silver),   fmt::format(" {} ", network ) );
            fmt::print(fg(fmt::color::aqua),    "⌚"); // ⏱ ◷ ⌚
            fmt::print(fg(fmt::color::silver), fmt::format("  {:%H:%Mm}    ", fmt::gmtime(now - _startedTime) ));
            fmt::print(fg(fmt::color::navy), " {:>{}}", ipport, 21 );
            fmt::print(fg(fmt::color::silver), "  │\n");
            fmt::print(fg(fmt::color::silver), " •───────────────────────────────────────────────────────────────────────•\n"); // ▫    
        }
    };     
}

GameServer::Server sv = GameServer::Server();

void clientThreadFunction( int clientID ) {
    bool connected = true;
    std::time_t tick = std::time(nullptr);
    std::time_t now = std::time(nullptr);
    GameServer::Client cli = sv.getClient( clientID );

    while( connected ) {
        std::string msg = sv.recvMsg( cli.sock );
        // All client logic goes here.
        Logger::serverInfo( fmt::format("Message from client {}: {}", cli.id, msg) );
        sv.consoleUpdate();

        // Test client connection after sometime.
        now = std::time(nullptr);
        if ( (now - tick) > 1 ) {
            tick = std::time(nullptr);
            int r = recv(cli.sock, NULL, 0, 0);
            if((r == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)) {
                connected = false;
                cli.isConnected = false;
            }
        }
    }
    Logger::serverInfo( fmt::format("Client {} disconnected ", cli.id) );
    sv.consoleUpdate();
    std::lock_guard<std::mutex> lock(iomutex);
    sv.disconnectedClients.push_back( cli.id );
}

void gameLogicThread() {
    Ticker tick = Ticker();

    Logger::serverInfo( "Hi from logic thread" );
    sv.consoleUpdate();

    /*int s = 0;
    while (s < 10) {
        double time_span = tick.compare();
        if( time_span > 1 ) {
            tick.update();
            Logger::serverInfo( fmt::format("{:1.2}", time_span ) );
            sv.consoleUpdate();
            s++;
        }
    }*/

    Logger::serverInfo( "Bye from logic thread" );
    sv.consoleUpdate();
}


int main() {
    _setmode(_fileno(stdout), _O_U8TEXT); 
    std::thread gameLogic(gameLogicThread);   
    
    while( sv.getStatus() ) {
        GameServer::Client cli;
        if( sv.acceptCon( cli ) ) {
            // Situação 1: IP já logado, desde que não esteja em modo debug.
            if( sv.isIPConnected(cli.ip) && !sv.isDebugMode() ) {
                Logger::serverWarning( fmt::format("Client at IP {} not connected, reason: ALREADY CONNECTED.", cli.ip) );
                sv.consoleUpdate();
                sv.sendMsg( cli, "Error, you are already connected!\n" );
            // Situação 2: Servidor cheio.
            } else if ( sv.isServerFull() ) {
                sv.addRefusedConnection();
                Logger::serverWarning( fmt::format("Client at IP {} not connected, reason: SERVER FULL.", cli.ip) );
                sv.consoleUpdate();
                sv.sendMsg( cli, "Error, server is full!\n" );
            // Situação 3: O IP foi banido do servidor.
            } else if ( sv.isBannedIP( cli ) ) {
                Logger::serverWarning( fmt::format("Client at IP {} not connected, reason: BANNED.", cli.ip) );
                sv.consoleUpdate();
                sv.sendMsg( cli, "Error, your IP was banned from server!\n" );
            } else {
                sv.sendMsg( cli, "Connection accepted.\n" );
                if( sv.addClient( cli ) ) {
                    Logger::serverInfo(fmt::format("Client {}:{} connected.", cli.ip, cli.port));
                    sv.consoleUpdate();
                } else {
                    Logger::serverError(fmt::format("Client {}:{} not connected.", cli.ip, cli.port));
                    sv.consoleUpdate();
                }
            }
        } else {
            Logger::serverError("Failed accepting connections.");
            sv.consoleUpdate();
        }
    }
    
    return 0;
}
