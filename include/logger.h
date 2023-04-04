#ifndef _SERVER_LOGGER_
#define _SERVER_LOGGER_

#include <Windows.h>
#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <fstream>

#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/color.h"
#include "fmt/chrono.h"

/* Estas linhas garantem que as cores das letras aparecerão em qualquer console do Windows. */
HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
DWORD dwMode = 0;
bool boolConsoleModeA = GetConsoleMode(hOut, &dwMode);
bool boolConsoleModeB = SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

namespace Logger {
    std::deque<std::vector<std::string>> messages;
    int messageCounter = 1;
    int MAX_DEQUE_SIZE = 10;
    std::string fileName = "serverlog.txt";

    void writeEntryToLogFile( std::vector<std::string> msg ) {
        std::string tag = "";
        if ( msg[0] == "success" ) {
            tag = "OKK";
        } else if ( msg[0] == "info" ) {
            tag = "INF";
        } else if ( msg[0] == "error" ) {
            tag = "ERR";
        } else if ( msg[0] == "warning" ) {
            tag = "WNG";
        } else {
            tag = "UKN";
        }
        std::ofstream file(fileName, std::ios_base::app);
        file << fmt::format("[{}] [{}] {}\n", msg[2], tag, msg[1] );
        file.close();
    }

    void startLoggingToFile() {
        std::time_t t = std::time(nullptr);
        std::ofstream file(fileName, std::ios_base::app);
        file << "\n----------------------------------------------------------------\n";
        file << fmt::format("Log started at: {:%d-%m-%y}, {:%H:%M:%S}\n", fmt::localtime(t), fmt::localtime(t));
        file.close();
    }

    void addToQueue( std::string type, std::string text ) {
        std::vector<std::string> msg;
        msg.push_back(type);
        msg.push_back(text);
        msg.push_back(std::to_string(messageCounter++));
        writeEntryToLogFile( msg );
        if( messages.size() == MAX_DEQUE_SIZE ) {
            messages.pop_front();
            messages.push_back( msg );
        } else {
            messages.push_back( msg );
        }
    }

    void printQueue() {
        for( int w=0; w < messages.size(); w++ ) {
            if (messages[w][0] == "success") {
                fmt::print(fg(fmt::color::black), fmt::format("[{:0>2}]", messages[w][2]) );
                fmt::print("> ");
                fmt::print(fg(fmt::color::lime), "✓");
                fmt::print(" {}\n", messages[w][1]);
            } else if ( messages[w][0] == "info") {
                fmt::print(fg(fmt::color::black), fmt::format("[{:0>2}]", messages[w][2]) );
                fmt::print("> ");
                fmt::print(fg(fmt::color::medium_blue), "!");
                fmt::print(" {}\n", messages[w][1]);
            } else if ( messages[w][0] == "error" ) {
                fmt::print(fg(fmt::color::black), fmt::format("[{:0>2}]", messages[w][2]) );
                fmt::print("> ");
                fmt::print(fg(fmt::color::red), "✕");
                fmt::print(" {}\n", messages[w][1]);
            } else {
                fmt::print(fg(fmt::color::black), fmt::format("[{:0>2}]", messages[w][2]) );
                fmt::print("> ");
                fmt::print(fg(fmt::color::yellow), "⚠");
                fmt::print(" {}\n", messages[w][1]);
            }
        }
    }

    static void serverSuccess( std::string message ) {
        addToQueue( "success", message );
        /*fmt::print("> ");
        fmt::print(fg(fmt::color::lime), "✓");
        fmt::print(" {}\n", message);*/
    }

    static void serverError( std::string message ) {
        addToQueue( "error", message );
        /*fmt::print("> ");
        fmt::print(fg(fmt::color::red), "✕");
        fmt::print(" {}\n", message);*/
    }

    static void serverInfo( std::string message ) {
        addToQueue( "info", message );
        /*fmt::print("> ");
        fmt::print(fmt::emphasis::bold | fg(fmt::color::blue), "!");
        fmt::print(" {}\n", message);*/
    }

    static void serverWarning( std::string message ) {
        addToQueue( "warning", message );
        /*fmt::print("> ");
        fmt::print(fmt::emphasis::bold | fg(fmt::color::yellow), "!");
        fmt::print(" {}\n", message);*/
    }

}

#endif