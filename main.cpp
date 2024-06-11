#include <iostream>
#include "defs.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "utils.hpp"
#include "perft.hpp"
#include <conio.h>

using namespace std;


int main() {
    // startFen =  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"; 
    board->parseFen(startFen);
    board->print();

    // perftTest(4);
    return 0;
}