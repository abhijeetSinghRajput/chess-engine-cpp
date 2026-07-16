#include <iostream>
#include <string>

#include "core/uci.hpp"
#include "core/utils.hpp"
#include "core/polyglot.hpp"

using namespace std;

#include "core/board.hpp"


int main()
{
    // string fine_70 = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -";
    // string WAC_2 = "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -";
    // string LCT_1 = "r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -";
    string path = "books/performance.bin";
    newGame();
    loadPolyBook(path);
    UCI();
    return 0;
}
