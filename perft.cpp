#include "defs.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "board.hpp"
#include <vector>
#include <iomanip>
#include <iostream>

U64 perft(int depth)
{
    if (depth <= 0) return 1;
    std::vector<std::pair<int,int>> moves = generateMoves();
    U64 count = 0;
    for (auto& pair : moves)
    {
        int move = pair.first;
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
    std::vector<std::pair<int,int>> moves = generateMoves();
    printf("go to perft %d\n", depth);
    long long startTime = getCurrTime();
    for (auto &pair : moves)
    {
        int move = pair.first;
        if (makeMove(move) == false)
        {
            continue;
        }

        int count = perft(depth - 1);
        takeMove();

        totalNodeSeared += count;
        std::cout << std::setw(2) << ++moveCount << " " << moveStr(move) << " " << count <<" " << getCurrTime() - startTime <<" ms" << std::endl;
    }
    std::cout << "Nodes searched: " << totalNodeSeared << std::endl;
    std::cout << "Total time: " << getCurrTime() - startTime;
}
