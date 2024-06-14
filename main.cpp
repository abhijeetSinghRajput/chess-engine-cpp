#include <iostream>
#include "defs.hpp"
#include "board.hpp"
#include "search.hpp"
#include "movegen.hpp"
#include "perft.hpp"

#include <conio.h>
using namespace std;

int main()
{
    // string maxMoves1 = "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1";
    // string maxMoves2 = "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1";
    // string perft1 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
    // string perft2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
    // string perft3 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
    string perft4 = "5rk1/1p6/q2P3p/2p2rp1/p1nbQ3/P1N3BP/1PR1B1P1/4K3 b - - 0 1";
    board->parseFen(perft4);
    board->print();
    searchPosition(10000);
    cout << moveStr(searchController->bestMove) << endl;

    cout<<"press any key to search again.";
    while (true)
    {
        char ch = getch();
        if (ch == 'q')
            break;
        system("cls");
        searchPosition(10000);
        cout << moveStr(searchController->bestMove) << endl;
    }

    return 0;
}