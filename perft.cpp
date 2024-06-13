#include "defs.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "board.hpp"
#include <vector>
#include <iostream>

U64 perft(int depth)
{
    if (depth <= 0) return 1;
    std::vector<int> moves = generateMoves();
    U64 count = 0;
    for (int move : moves)
    {
        if(move <= 0){
            printf("move tree\n");
            for(int i = 0; i<board->ply; ++i){
                printf("%d\n", board->history[i]->move);
            }
            printf("gen moves:\n");
            for(int move : moves){
                printf("%d\n", move);
            }
        }
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
    std::vector<int> moves = generateMoves();
    printf("go to perft %d\n", depth);
    for (int move : moves)
    {
        if (makeMove(move) == false)
        {
            continue;
        }

        int count = perft(depth - 1);
        takeMove();

        totalNodeSeared += count;
        printf("%2d %s %d\n", ++moveCount, moveStr(moveFrom(move), moveTo(move)), count);
    }
    std::cout << "Total Node Seared " << totalNodeSeared << std::endl;
}
