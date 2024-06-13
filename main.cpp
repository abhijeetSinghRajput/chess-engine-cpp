#include <iostream>
#include "defs.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "utils.hpp"
#include "perft.hpp"
#include "movegen.hpp"
#include "evaluation.hpp"
#include <conio.h>

using namespace std;

int main()
{
    startFen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
    startFen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    board->parseFen(startFen);
    board->print();
    printf("eval : %d\n", evalPosition());
    return 0;
}