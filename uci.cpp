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

void handleSearch(std::istringstream &iss)
{
    std::string searchOption;
    iss >> searchOption;

    if (searchOption == "depth")
    {
    }
    else if (searchOption == "movetime")
    {
    }
    else if (searchOption == "infinite")
    {
    }
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
            searchController->stop = true;
        }
        else if (command == "quit")
        {
            break;
        }
    }
}
