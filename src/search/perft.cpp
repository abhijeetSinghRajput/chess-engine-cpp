#include "core/defs.hpp"
#include "core/move.hpp"
#include "core/movegen.hpp"
#include "core/board.hpp"
#include <vector>
#include <iomanip>
#include <iostream>

U64 perft(int depth)
{
    if (depth <= 0) return 1;

    MoveList moves = generateMoves();
    U64 count = 0;
       for (int i = 0; i < moves.count; ++i)
    {
        int move = moves.moves[i].move;
        if (makeMove(move) == false)
            continue;
            
        count += perft(depth - 1);
        takeMove();
    }
    return count;
}

void perftTest(int depth)
{
    int moveCount = 0;
    U64 totalNodeSeared = 0;
    MoveList moves = generateMoves();
    long long startTime = getCurrTime();
    for (int i = 0; i < moves.count; ++i)
    {
        int move = moves.moves[i].move;
        if (makeMove(move) == false)
        {
            continue;
        }

        int count = perft(depth - 1);
        takeMove();

        totalNodeSeared += count;
        std::cout << std::setw(2)
                  << ++moveCount << " "
                  << moveStr(move) << " "
                  << count << "\t"
                  << getCurrTime() - startTime << " ms"
                  << std::endl;
    }

    std::cout << "\nNodes searched: " << totalNodeSeared << std::endl;
    std::cout << "Total time: " << getCurrTime() - startTime << std::endl;
}
