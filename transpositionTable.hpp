#pragma once

#include "defs.hpp"

extern const int AlphaFlag;
extern const int BetaFlag;
extern const int ExactFlag;

class TranspositionTable
{
private:
    static const int maxEntries = 0x100000 * 16;
    TableData entries[maxEntries];
    int currentAge;    // increments each search

public:
    TranspositionTable();
    ~TranspositionTable();
    void clear();
    void newSearch();  // call at start of each search
    void add(U64 positionKey, int move, int score, int flag, int depth);
    TableData *get(U64 positionKey);
    std::vector<int> getLine(int depth);
    int getMove();
};

inline U64 pack_tableData(int score, int depth, int flag, U64 move)
{
    return (U64)(score + INFINITE)
         | ((U64)(depth & 0x3f) << 16)
         | ((U64)(flag & 0x3) << 22)
         | (move << 24);
}

inline int extract_score(U64 data) { return (int)(data & 0xffff) - INFINITE; }
inline int extract_move(U64 data) { return data >> 24; }
inline int extract_flag(U64 data) { return (data >> 22) & 3; }
inline int extract_depth(U64 data) { return (data >> 16) & 0x3f; }

extern TranspositionTable* transpositionTable;