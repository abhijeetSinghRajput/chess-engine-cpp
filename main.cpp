#include <iostream>
#include <string>

#include "uci.hpp"
#include "utils.hpp"
#include "polyglot.hpp"

using namespace std;

#include "board.hpp"

// uint64_t key = 0x463b96181691fc9c;

// uint16_t moves[3] = {5310, 1234, 5678};
// uint16_t weights[3] = {8012, 3456, 7890};
// uint32_t learns[3] = {0, 1, 2};

int main()
{
    // string fine_70 = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -";
    // string WAC_2 = "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -";
    // string LCT_1 = "r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -";
    string path = "./performance.bin";
    newGame();
    loadPolyBook(path);
    UCI();
    return 0;
}

//  Fen: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
//  Castle: 15
// Depth    Time      Ordering     Nodes     Best Move   Best Score   Line
// 1        4          0.00        21          d2d4           30      d2d4
// 2        5          86.36       92          d2d4           0       d2d4 d7d5
// 3        7          82.35       722         d2d4           25      d2d4 d7d5 c1e3
// 4        10         84.36       3658        d2d4           0       d2d4 d7d5 c1e3 c8f5
// 5        17         83.26       12197       e2e4           25      e2e4
// 6        71         79.87       84773       e2e4           5       e2e4
// 7        188        81.29       243331      e2e4           22      e2e4
// 8        1211       83.31       2230778     e2e4           5       e2e4
// 9        2880       84.24       5804530     e2e4           25      e2e4
// 10       19727      85.17       42211467    e2e4           10      e2e4

