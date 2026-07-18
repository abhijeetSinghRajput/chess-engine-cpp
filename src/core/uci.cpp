#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include "core/uci.hpp"
#include "core/utils.hpp"
#include "core/board.hpp"
#include "core/move.hpp"
#include "core/polyglot.hpp"
#include "eval/evaluation.hpp"
#include "search/search.hpp"
#include "search/transpositionTable.hpp"

std::thread searchThread;

void handlePosition(std::istringstream &iss)
{
    std::string positionType, token;
    std::string fen = "";

    iss >> positionType;
    
    if (positionType == "startpos")
    {
        board->parseFen(START_FEN);
    }
    else if (positionType == "fen")
    {
        // Read exactly 6 FEN fields
        for (int i = 0; i < 6; i++) {
            std::string field;
            iss >> field;
            fen += field;
            if (i < 5) fen += " ";
        }
        
        // std::cout << "fen : " << fen << std::endl; // todo 
        board->parseFen(fen);
    }

    // Now read remaining tokens (should be "moves" if present)
    while (iss >> token)
    {
        if (token == "moves")
        {
            while (iss >> token)
            {
                int move = parseMove(token);
                if (!move) break;
                makeMove(move);
            }
            break;  // No need to process more after "moves"
        }
    }
}

void handleSearch(std::istringstream &iss)
{
    int depth = MAX_DEPTH, time = 0, inc = 0, movetime = 0, movestogo = 30;
    std::string token;

    while (iss >> token)
    {
        if (token == "depth")
            iss >> depth;
        else if (token == "movetime")
            iss >> movetime;
        else if (token == "movestogo")
            iss >> movestogo;
        else if (board->side == WHITE && token == "wtime")
            iss >> time;
        else if (board->side == BLACK && token == "btime")
            iss >> time;
        else if (board->side == WHITE && token == "winc")
            iss >> inc;
        else if (board->side == BLACK && token == "binc")
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
        int perMove = time / movestogo;

        // Safety cap: never allocate more than 70% of the RAW remaining clock
        // to a single move, no matter how small movestogo is (e.g. movestogo=1).
        int maxAllowed = static_cast<int>(time * 0.7);
        perMove = std::min(perMove, maxAllowed);

        constexpr int MOVE_OVERHEAD = 50;
        perMove -= MOVE_OVERHEAD;
        if (perMove < 1)
            perMove = 1;

        time = perMove;
    }

    // std::cout<<"time: " <<time <<std::endl;
    // std::cout<<"startTime: " <<searchController->startTime <<std::endl;
    // std::cout<<"stopTime: " <<searchController->stopTime <<std::endl;
    // std::cout<<"timeSet: " <<searchController->timeSet <<std::endl;

    if (searchThread.joinable())
    {
        searchController->stopped = true;
        searchThread.join();
        searchController->stopped = false;
    }

    searchController->stopTime = searchController->startTime + time;
    searchThread = std::thread(searchPosition);
}

void handleOptions(std::istringstream &iss)
{
    std::string token;
    std::string optionName;
    std::string optionValue;

    while (iss >> token)
    {
        if (token == "name")
        {
            iss >> optionName;
        }
        else if (token == "value")
        {
            iss >> optionValue;
        }
    }
    if (optionName == "usebook")
    {
        searchController->useBook = (optionValue == "true") ? true : false;
        if (searchController->useBook)
        {
            printf("book move on\n");
        }
        else{
            printf("book move off\n");
        }
    }
    else if ( optionName == "Hash"){
        int mb = std::stoi(optionValue);
        transpositionTable->resize(mb);
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
            std::cout << "option name Hash type spin default 16 min 1 max 4096\n";
            std::cout << "option name usebook type check default " 
                      << (searchController->useBook ? "true" : "false") 
                      << std::endl;

            std::cout << "uciok" << std::endl;
        }
        else if (command == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        else if (command == "setoption")
        {
            handleOptions(iss);
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
            }
            makeMove(move);
            board->print();

            // engine move
            if(isGameOver()){
                if(board->checkSq != SQ_NONE){
                    printf("Game over by Checkmate\n");
                }
                else{
                    printf("Draw\n");
                }
                continue;
            }

            searchController->depth = MAX_DEPTH;
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
                printf("engine is unable to think\n");
            }
        }
        else if (command == "undo")
        {
            takeMove();
        }
        else if (command == "book")
        {
            readBook();
        }
        else if(command == "eval"){
            printf("Eval: %d\n", evalPosition());
        }
        else if(command == "cls" || command == "clear"){
            system("cls");
            printf("> ");
        }
    }
}
