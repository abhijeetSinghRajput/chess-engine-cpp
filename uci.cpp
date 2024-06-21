#include "uci.hpp"
#include "utils.hpp"
#include "board.hpp"
#include "search.hpp"
#include "move.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

std::thread searchThread;

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
                if (!move)
                    break;
                makeMove(move);
            }
        }
    }

    board->print();
}

void handleSearch(std::istringstream &iss)
{
    int depth = maxDepth, time = 0, inc = 0, movetime = 0, movestogo = 30;
    std::string token;

    while (iss >> token)
    {
        if (token == "depth")
            iss >> depth;
        else if (token == "movetime")
            iss >> movetime;
        else if (token == "movestogo")
            iss >> movestogo;
        else if (board->side == white && token == "wtime")
            iss >> time;
        else if (board->side == black && token == "btime")
            iss >> time;
        else if (board->side == white && token == "winc")
            iss >> inc;
        else if (board->side == black && token == "binc")
            iss >> inc;
    }

    searchController->depth = depth;
    searchController->startTime = getCurrTime();

    if (movetime > 0)
    {
        searchController->timeSet = true;
        time = movetime;
    }
    else
    {
        time += inc;
        searchController->timeSet = static_cast<bool>(time);

        if (movestogo <= 0)
        {
            movestogo = 1; // Avoid division by zero
        }
        time /= movestogo;
    }

    // std::cout<<"time: " <<time <<std::endl;
    // std::cout<<"startTime: " <<searchController->startTime <<std::endl;
    // std::cout<<"stopTime: " <<searchController->stopTime <<std::endl;
    // std::cout<<"timeSet: " <<searchController->timeSet <<std::endl;

    if (searchThread.joinable())
    {
        searchThread.join();
    }

    searchController->stopTime = searchController->startTime + time;
    searchThread = std::thread(searchPosition);
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
            if (searchThread.joinable())
            {
                searchController->stopped = true;
                searchThread.join();
            }
        }
        else if (command == "quit")
        {
            if (searchThread.joinable())
            {
                searchController->stopped = true;
                searchThread.join();
            }
            break;
        }
        else if (command == "d")
        {
            board->print();
        }
        else if (command == "move")
        {
            std::string token;
            iss >> token;
            int move = parseMove(token);
            if (!move)
            {
                printf("enter a valid move\n");
                continue;
                ;
            }
            makeMove(move);
            board->print();

            // engine move
            searchController->depth = maxDepth;
            searchController->startTime = getCurrTime();
            searchController->stopTime = searchController->startTime + 2000;
            searchController->timeSet = true;

            int engineMove = searchPosition();
            if (engineMove)
            {
                makeMove(engineMove);
                board->print();
            }
            else
            {
                printf("engine is unable think\n");
            }
        }
        else if (command == "undo")
        {
            takeMove();
        }
    }
}
