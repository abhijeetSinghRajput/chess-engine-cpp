// perft.cpp
#include "core/defs.hpp"
#include "core/move.hpp"
#include "core/movegen.hpp"
#include "core/board.hpp"
#include <vector>
#include <iomanip>
#include <iostream>
#include <locale>

// Custom facet so integers/doubles print with comma thousands-separators
struct comma_numpunct : std::numpunct<char>
{
protected:
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

U64 perft(int depth)
{
    if (depth <= 0) return 1;

    MoveList moves;
    generateMoves(moves);
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
    // Imbue cout once with the comma locale for this function's output
    static std::locale commaLocale(std::locale(), new comma_numpunct());
    std::cout.imbue(commaLocale);

    int moveCount = 0;
    U64 totalNodeSeared = 0;
    MoveList moves;
    generateMoves(moves);
    long long startTime = getCurrTime();
    for (int i = 0; i < moves.count; ++i)
    {
        int move = moves.moves[i].move;
        if (makeMove(move) == false)
        {
            continue;
        }

        U64 count = perft(depth - 1);
        takeMove();

        totalNodeSeared += count;
        std::cout << std::setw(2)
                  << ++moveCount << " "
                  << moveStr(move) << " "
                  << count << "\t"
                  << getCurrTime() - startTime << " ms"
                  << std::endl;
    }

    long long totalTimeTaken = getCurrTime() - startTime;
    double nps = totalTimeTaken > 0
        ? (static_cast<double>(totalNodeSeared) / totalTimeTaken) * 1000.0
        : 0.0;

    std::cout << "\nNodes searched: " << totalNodeSeared << std::endl;
    std::cout << "Total time: " << totalTimeTaken << " ms" << std::endl;
    std::cout << "NPS: " << std::fixed << std::setprecision(2) << nps << std::endl;
    std::cout << std::endl;
}