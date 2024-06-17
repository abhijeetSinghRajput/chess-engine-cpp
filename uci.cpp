#include "uci.hpp"
#include "utils.hpp"
#include "board.hpp"
#include "search.hpp"
#include "move.hpp"
#include <iostream>
#include <sstream>
#include <vector>

void handlePosition(std::istringstream &iss)
{
    std::string positionType, fen, token;

    iss >> positionType;
    if (positionType == "startpos")
    {
        board->parseFen(startFen);
    }
    else if (positionType == "fen")
    {
        getline(iss, fen);
        std::cout << "fen : " << fen << std::endl;
        board->parseFen(fen);
    }

    while (iss >> token)
    {
        if (token == "moves")
        {
            while (iss >> token)
            {
                int move = parseMove(token);
                if(!move) break;
                makeMove(move);
                std::cout<<token<<std::endl;
            }
        }
    }

    board->print();
}

void handleSearch(std::istringstream &iss) {
    int depth = maxDepth, time = 0, inc = 0, movetime = 0, movestogo = 30;
    std::string token;

    while (iss >> token) {
        if (token == "depth") iss >> depth;
        else if (token == "movetime") iss >> movetime;
        else if (token == "movestogo") iss >> movestogo;
        else if (board->side == white && token == "wtime") iss >> time;
        else if (board->side == black && token == "btime") iss >> time;
        else if (board->side == white && token == "winc") iss >> inc;
        else if (board->side == black && token == "binc") iss >> inc;
    }

    searchController->depth = depth;
    searchController->startTime = getCurrTime();

    if (movetime > 0) {
        searchController->timeSet = true;
        time = movetime;
    } else {
        time += inc;
        searchController->timeSet = static_cast<bool>(time);

        if (movestogo <= 0) {
            movestogo = 1;  // Avoid division by zero
        }
        time /= movestogo;
    }
    
    // std::cout<<"time: " <<time <<std::endl;
    // std::cout<<"startTime: " <<searchController->startTime <<std::endl;
    // std::cout<<"stopTime: " <<searchController->stopTime <<std::endl;
    // std::cout<<"timeSet: " <<searchController->timeSet <<std::endl;

    searchController->stopTime = searchController->startTime + time;
    searchPosition();
}


void UCI()
{
    std::string input, command;
    std::string positionType, fen;

    while (true)
    {
        std::getline(std::cin, input);
        std::istringstream iss(input);
        iss >> command;
        if (command == "uci")
        {
            std::cout << "id name " << name << std::endl;
            std::cout << "id author " << author << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (command == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        else if (command == "setoption")
        {
        }
        else if (command == "ucinewgame")
        {
            newGame();
        }
        else if (command == "position")
        {
            handlePosition(iss);
        }
        else if (command == "go")
        {
            handleSearch(iss);
        }
        else if (command == "stop")
        {
            searchController->stopped = true;
        }
        else if (command == "quit")
        {
            searchController->stopped = true;
            break;
        }
        else if(command == "d"){
            board->print();
        }
    }
}
