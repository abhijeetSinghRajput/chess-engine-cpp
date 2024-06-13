#pragma once
#include "defs.hpp"

extern const int AlphaFlag;
extern const int BetaFlag;
extern const int ExactFlag;

class TranspositionTable
{
private:
    static const int maxEntries = 0x100000 * 16;
    TableData *entries[maxEntries];

public:
    TranspositionTable();
    void clear();
    void add(U64 positionKey, int move, int score, int flag, int depth);
    TableData *get(U64 positionKey);
    std::vector<int> &getLine(int depth);
    int getMove();
};

extern TranspositionTable* transpositionTable;
