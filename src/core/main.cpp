// main.cpp
#include <iostream>
#include <string>

#include "core/uci.hpp"
#include "core/utils.hpp"
#include "core/polyglot.hpp"

using namespace std;

#include "core/board.hpp"


int main(int argc, char* argv[])
{
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))
    {
        printHelp();
        return 0;
    }
    cout << "=============================================="  << endl;
    cout << " Chanakya v1.0.0 - UCI Chess Engine"             << endl;
    cout << " by Abhijeet Singh Rajput (Mr. Codium)"          << endl;
    cout << "=============================================="  << endl;
    cout << "Type 'help' for commands, 'uci' for UCI mode.\n" << endl;

    string path = "books/performance.bin";
    newGame();
    loadPolyBook(path);
    UCI();
    return 0;
}
