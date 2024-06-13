#pragma once
#include "defs.hpp"

class SearchController
{
private:
public:
    int depthReached;
    int nodes;
    int fh;
    int fhf;
    int depth;
    int time;
    int start;
    bool stop;
    int bestMove;
    int bestScore;
    bool thinking;
    int ply;
    int killers[64][2];   // killer[ply][0/1];
    int history[13][120]; // history[piece][toSq]

    void clear();
};

int searchPosition(int thinkingTime);
int alphaBeta(int alpha, int beta, int depth, bool doNull);
int quiescence(int alpha, int beta);
int swapWithBest(int i, int move);
bool checkTimeUp();
bool isRepetition();

extern SearchController *searchController;
