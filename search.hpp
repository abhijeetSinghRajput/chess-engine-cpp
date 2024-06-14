#pragma once
#include "defs.hpp"

class SearchController
{
private:
public:
    int depthReached;
    long long nodes;
    int fh;
    int fhf;
    int depth;
    long long time;
    long long start;
    bool stop;
    int bestMove;
    int bestScore;
    bool thinking;
    int ply;
    int killers[64][2];   // killer[ply][0/1];
    int history[13][120]; // history[piece][toSq]

    void clear();
};

void searchPosition(long long thinkingTime = 2000);
int alphaBeta(int alpha, int beta, int depth, bool doNull);
int quiescence(int alpha, int beta);
void swapWithBest(int i, std::vector<std::pair<int, int>> &moves);
void checkTimeUp();
bool isRepetition();

extern SearchController *searchController;
